#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include <filesystem>
#include <regex>

#include <jankzip.h>

#include <jank/type.hpp>
#include <jank/util/fmt/print.hpp>
#include <jank/util/path.hpp>
#include <jank/runtime/core.hpp>
#include <jank/runtime/core/munge.hpp>
#include <jank/runtime/core/truthy.hpp>
#include <jank/runtime/obj/persistent_array_map.hpp>
#include <jank/detail/to_runtime_data.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/obj/atom.hpp>
#include <jank/runtime/obj/jit_function.hpp>
#include <jank/runtime/obj/native_function_wrapper.hpp>
#include <jank/runtime/obj/persistent_sorted_set.hpp>
#include <jank/runtime/obj/persistent_hash_map.hpp>
#include <jank/runtime/module/loader.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/util/environment.hpp>
#include <jank/profile/time.hpp>
#include <jank/error/runtime.hpp>

namespace jank::runtime::module
{
  using zip_ptr = std::unique_ptr<zip_t, decltype(&zip_close)>;
  using zip_entry_ptr = std::unique_ptr<zip_t, decltype(&zip_entry_close)>;

  /* This turns `foo_bar/spam/meow.cljc` into `foo-bar.spam.meow`. */
  jtl::immutable_string path_to_module(std::filesystem::path const &path)
  {
    static std::regex const slash{ "/" };

    auto const &s(runtime::demunge(path.native()));
    std::string ret{ s, 0, s.size() - path.extension().native().size() };

    /* There's a special case of the / function which shouldn't be treated as a path. */
    if(ret.find("$/") == std::string::npos)
    {
      ret = std::regex_replace(ret, slash, ".");
    }

    return ret;
  }

  jtl::immutable_string module_to_path(jtl::immutable_string const &module)
  {
    static jtl::immutable_string const dot{ "\\." };
    return runtime::munge_and_replace(module, dot, "/");
  }

  jtl::immutable_string module_to_load_function(jtl::immutable_string const &module)
  {
    static jtl::immutable_string const dot{ "\\." };
    auto const &ret{ runtime::munge_and_replace(module, dot, "_") };

    return util::format("jank_load_{}", ret);
  }

  /* TODO: I don't think this is needed. */
  jtl::immutable_string
  nest_module(jtl::immutable_string const &module, jtl::immutable_string const &sub)
  {
    jank_debug_assert(!module.empty());
    jank_debug_assert(!sub.empty());
    return module + "$" + sub;
  }

  jtl::immutable_string
  nest_native_ns(jtl::immutable_string const &native_ns, jtl::immutable_string const &end)
  {
    jank_debug_assert(!native_ns.empty());
    jank_debug_assert(!end.empty());
    return util::format("::{}::{}", native_ns, end);
  }

  /* If it has two or more occurences of $, it's nested. */
  bool is_nested_module(jtl::immutable_string const &module)
  {
    return module.find('$') != module.rfind('$');
  }

  /* This is a somewhat complicated function. We take in a module (doesn't need to be munged) and
   * we return a native namespace name. So foo.bar will become foo::bar. But we also strip off
   * the last nested module, since the way the codegen works is that foo.bar$spam lives in the
   * native namespace foo::bar. Lastly, we need to split the module into parts and munge each
   * individually, since we can have a module like clojure.template which will munge cleanly
   * on its own, but template is a C++ keyword and the resulting clojure::template namespace
   * will be a problem. So we split the module on each ., munge, and put it back together
   * using ::. */
  jtl::immutable_string module_to_native_ns(jtl::immutable_string const &orig_module)
  {
    static std::regex const dollar{ "\\$" };

    native_transient_string module{ munge(orig_module) };

    native_vector<native_transient_string> module_parts;
    for(size_t dot_pos{}; (dot_pos = module.find('.')) != jtl::immutable_string::npos;)
    {
      module_parts.emplace_back(munge(module.substr(0, dot_pos)));
      module.erase(0, dot_pos + 1);
    }

    if(module.find('$') != native_transient_string::npos)
    {
      for(size_t dollar_pos{}; (dollar_pos = module.find('$')) != jtl::immutable_string::npos;)
      {
        module_parts.emplace_back(munge(module.substr(0, dollar_pos)));
        module.erase(0, dollar_pos + 1);
      }
    }
    else
    {
      module_parts.emplace_back(munge(module));
    }

    std::string ret;
    for(auto &part : module_parts)
    {
      part = std::regex_replace(part, dollar, "::");

      if(!ret.empty())
      {
        ret += "::";
      }
      ret += part;
    }

    return ret;
  }

  static native_set<jtl::immutable_string> const &core_modules()
  {
    static native_set<jtl::immutable_string> const modules{ "clojure.core" };
    return modules;
  }

  bool is_core_module(jtl::immutable_string const &module)
  {
    return core_modules().contains(module);
  }

  static zip_entry_ptr open_zip_entry(zip_t * const zip, jtl::immutable_string const &path)
  {
    zip_entry_open(zip, path.c_str());
    return { zip, &zip_entry_close };
  }

  static zip_entry_ptr open_zip_entry(zip_t * const zip, ssize const index)
  {
    zip_entry_openbyindex(zip, index);
    return { zip, &zip_entry_close };
  }

  static jtl::result<jtl::immutable_string, error_ref> read_zip_entry(zip_t * const zip)
  {
    auto const entry_size{ zip_entry_size(zip) };
    jtl::string_builder sb;
    sb.reserve(entry_size);
    auto const read_result{
      zip_entry_noallocread(zip, reinterpret_cast<void *>(sb.data()), entry_size)
    };
    if(read_result < 0)
    {
      auto const entry_name{ zip_entry_name(zip) };
      return error::internal_runtime_failure(
        util::format("Failed to read jar entry '{}' with error '{}'.",
                     entry_name,
                     zip_strerror(static_cast<int>(read_result))));
    }

    sb.pos = read_result;
    return sb.release();
  }

  template <typename F>
  static jtl::result<void, error_ref> visit_jar_entry(file_entry const &entry, F const &fn)
  {
    auto const &path(entry.archive_path.unwrap());

    int ziperr{};
    zip_ptr const zip{ zip_openwitherror(path.c_str(), 0, 'r', &ziperr), &zip_close };
    if(ziperr < 0)
    {
      return error::internal_runtime_failure(
        util::format("Failed to open jar '{}' with error '{}'.", path, zip_strerror(ziperr)));
    }

    auto const entry_handle{ open_zip_entry(zip.get(), entry.path.c_str()) };
    auto const res{ fn(zip.get()) };
    if(res.is_err())
    {
      return res.expect_err();
    }

    return ok();
  }

  static void register_entry(native_unordered_map<jtl::immutable_string, loader::entry> &entries,
                             std::filesystem::path const &module_path,
                             file_entry const &entry)
  {
    std::filesystem::path const p{ native_transient_string{ entry.path } };
    auto const ext(p.extension().native());
    bool registered{};
    if(ext == ".jank")
    {
      registered = true;
      loader::entry e;
      e.jank = entry;
      auto res(entries.insert({ path_to_module(module_path), std::move(e) }));
      if(!res.second)
      {
        res.first->second.jank = entry;
      }
    }
    else if(ext == ".cljc")
    {
      registered = true;
      loader::entry e;
      e.cljc = entry;
      auto res(entries.insert({ path_to_module(module_path), std::move(e) }));
      if(!res.second)
      {
        res.first->second.cljc = entry;
      }
    }
    else if(ext == ".cpp")
    {
      registered = true;
      loader::entry e;
      e.cpp = entry;
      auto res(entries.insert({ path_to_module(module_path), std::move(e) }));
      if(!res.second)
      {
        res.first->second.cpp = entry;
      }
    }
    else if(ext == ".o")
    {
      registered = true;
      loader::entry e;
      e.o = entry;
      auto res(entries.insert({ path_to_module(module_path), std::move(e) }));
      if(!res.second)
      {
        res.first->second.o = entry;
      }
    }

    if(registered)
    {
      //util::println("register_entry {} {} {} {}",
      //              entry.archive_path.unwrap_or("None"),
      //              entry.path,
      //              module_path.native(),
      //              path_to_module(module_path));
    }
  }

  static void
  register_relative_entry(native_unordered_map<jtl::immutable_string, loader::entry> &entries,
                          std::filesystem::path const &resource_path,
                          file_entry const &entry)
  {
    std::filesystem::path const p{ native_transient_string{ entry.path } };
    /* We need the file path relative to the module path, since the class
     * path portion is not included in part of the module name. For example,
     * the file may live in `src/jank/clojure/core.jank` but the module
     * should be `clojure.core`, not `src.jank.clojure.core`. */
    auto const &module_path(p.lexically_relative(resource_path));
    register_entry(entries, module_path, entry);
  }

  static void
  register_directory(native_unordered_map<jtl::immutable_string, loader::entry> &entries,
                     std::filesystem::path const &path)
  {
    for(auto const &f : std::filesystem::recursive_directory_iterator{ path })
    {
      if(std::filesystem::is_regular_file(f))
      {
        register_relative_entry(entries, path, file_entry{ none, f.path().native() });
      }
    }
  }

  static void register_jar(native_unordered_map<jtl::immutable_string, loader::entry> &entries,
                           jtl::immutable_string const &path)
  {
    int ziperr{};
    zip_ptr const zip{ zip_openwitherror(path.c_str(), 0, 'r', &ziperr), &zip_close };
    if(ziperr < 0)
    {
      //util::println(stderr, "Failed to open jar on module path: {}\n", path);
      return;
    }

    auto const entry_count{ zip_entries_total(zip.get()) };
    for(ssize i{}; i < entry_count; ++i)
    {
      auto const entry_handle{ open_zip_entry(zip.get(), i) };
      {
        auto const entry_name{ zip_entry_name(zip.get()) };
        auto const is_dir{ static_cast<bool>(zip_entry_isdir(zip.get())) };

        if(!is_dir)
        {
          register_entry(entries, entry_name, { path, entry_name });
        }
      }
    }
  }

  static void register_path(native_unordered_map<jtl::immutable_string, loader::entry> &entries,
                            jtl::immutable_string_view const &path)
  {
    /* It's entirely possible to have empty entries in the module path, mainly due to lazy string
     * concatenation. We just ignore them. This means something like "::::" is valid. */
    if(path.empty() || !std::filesystem::exists(std::string_view{ path }))
    {
      return;
    }

    std::filesystem::path const p{
      std::filesystem::canonical(std::string_view{ path }).lexically_normal()
    };
    if(std::filesystem::is_directory(p))
    {
      register_directory(entries, p);
    }
    else if(p.extension().native() == ".jar")
    {
      register_jar(entries, path);
    }
    /* If it's not a JAR or a directory, we just add it as a direct file entry. I don't think the
     * JVM supports this, but I like that it allows us to put specific files in the path. */
    else
    {
      auto const &module_path(p.native());
      register_entry(entries, module_path, { none, module_path });
    }
  }

  static void
  register_module_path(native_unordered_map<jtl::immutable_string, loader::entry> &entries,
                       jtl::immutable_string const &paths,
                       bool const skip_jar)
  {
    usize start{};
    usize i{ paths.find(loader::module_separator, start) };

    /* Looks like it's either an empty path list or there's only entry. */
    if(i == jtl::immutable_string::npos)
    {
      if(!skip_jar || !paths.ends_with(".jar"))
      {
        register_path(entries, paths);
      }
    }
    else
    {
      while(i != jtl::immutable_string::npos)
      {
        auto const path(paths.substr(start, i - start));

        if(!skip_jar || !path.ends_with(".jar"))
        {
          register_path(entries, path);
        }

        start = i + 1;
        i = paths.find(loader::module_separator, start);
      }

      auto const path(paths.substr(start, i - start));
      if(!skip_jar || !path.ends_with(".jar"))
      {
        register_path(entries, path);
      }
    }
  }

  loader::loader()
  {
    std::filesystem::path const jank_path{ jank::util::process_dir().c_str() };
    std::filesystem::path const resource_dir{ jank::util::resource_dir().c_str() };
    auto const binary_cache_dir{ util::binary_cache_dir(util::binary_version()) };
    native_transient_string paths{ util::cli::opts.module_path };

    /* These paths are used by an installed jank. */
    paths += util::format(":{}", binary_cache_dir);
    paths += util::format(":{}", (resource_dir / "src/jank").native());

    /* These paths below are only used during development. */
    paths += util::format(":{}", (jank_path / "core-libs").native());
    paths += util::format(":{}", (jank_path / binary_cache_dir.c_str()).native());
    paths += util::format(":{}", (jank_path / "../src/jank").native());

    this->paths = paths;

    //util::println("module paths: {}", paths);
    register_module_path(entries, paths, false);
  }

  object_ref file_entry::to_runtime_data() const
  {
    return runtime::obj::persistent_array_map::create_unique(
      make_box("__type"),
      make_box("module::file_entry"),
      make_box("archive_path"),
      jank::detail::to_runtime_data(archive_path),
      make_box("path"),
      make_box(path));
  }

  bool file_entry::exists() const
  {
    auto const is_archive{ archive_path.is_some() };
    if(is_archive && !std::filesystem::exists(native_transient_string{ archive_path.unwrap() }))
    {
      return false;
    }
    else
    {
      bool source_exists{};
      if(is_archive)
      {
        auto const res{ visit_jar_entry(*this,
                                        [&](zip_t * const zip) -> jtl::result<void, error_ref> {
                                          source_exists = static_cast<bool>(zip_entry_isdir(zip));
                                          return ok();
                                        }) };
        if(res.is_err())
        {
          return false;
        }
      }

      return source_exists || std::filesystem::exists(native_transient_string{ path });
    }
  }

  std::time_t file_entry::last_modified_at() const
  {
    auto const source_path{ archive_path.unwrap_or(path) };

    /* NOLINTNEXTLINE(*-narrowing-conversions) */
    return std::filesystem::last_write_time(native_transient_string{ source_path })
      .time_since_epoch()
      .count();
  }

  file_view::file_view(file_view &&mf) noexcept
    : fd{ mf.fd }
    , head{ mf.head }
    , len{ mf.len }
    , buff{ jtl::move(mf.buff) }
    , file{ jtl::move(mf.file) }
  {
    mf.fd = -1;
    mf.head = nullptr;
  }

  file_view::file_view(jtl::immutable_string const &file,
                       int const f,
                       char const * const h,
                       usize const s)
    : fd{ f }
    , head{ h }
    , len{ s }
    , file{ file }
  {
  }

  file_view::file_view(jtl::immutable_string const &file, jtl::immutable_string const &buff)
    : buff{ buff }
    , file{ file }
  {
  }

  file_view::~file_view()
  {
    reset();
  }

  file_view &file_view::operator=(file_view &&fv) noexcept
  {
    if(this == &fv)
    {
      return *this;
    }

    reset();
    fd = fv.fd;
    head = fv.head;
    len = fv.len;
    buff = jtl::move(fv.buff);
    file = jtl::move(fv.file);

    fv.fd = -1;
    fv.head = nullptr;

    return *this;
  }

  char const *file_view::data() const
  {
    return buff.empty() ? head : buff.data();
  }

  usize file_view::size() const
  {
    return buff.empty() ? len : buff.size();
  }

  jtl::immutable_string const &file_view::file_path() const
  {
    return file;
  }

  jtl::immutable_string_view file_view::view() const
  {
    return { data(), size() };
  }

  void file_view::reset()
  {
    if(head != nullptr)
    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast): I want const everywhere else. */
    {
      munmap(reinterpret_cast<void *>(const_cast<char *>(head)), len);
      head = nullptr;
    }
    if(fd >= 0)
    {
      ::close(fd);
      fd = -1;
    }
  }

  static jtl::result<file_view, error_ref> read_jar_file(jtl::immutable_string const &path)
  {
    using namespace runtime;
    using namespace runtime::module;

    auto const colon{ path.find(':') };
    auto const jar_path{ path.substr(0, colon) };
    auto const file_path{ path.substr(colon + 1) };
    auto const module{ path_to_module(std::string{ file_path.data(), file_path.size() }) };
    auto const found_module{ __rt_ctx->module_loader.find(module, origin::source) };
    if(found_module.is_err())
    {
      return err(found_module.expect_err());
    }

    int ziperr{};
    zip_ptr const zip{ zip_openwitherror(jar_path.c_str(), 0, 'r', &ziperr), &zip_close };
    if(ziperr < 0)
    {
      return error::internal_runtime_failure(
        util::format("Failed to open jar '{}' with error {}, '{}'.",
                     jar_path,
                     ziperr,
                     zip_strerror(ziperr)));
    }

    auto const entry_handle{ open_zip_entry(zip.get(), file_path) };
    auto const read_result{ read_zip_entry(zip.get()) };
    if(read_result.is_err())
    {
      return read_result.expect_err();
    }
    return ok(file_view{ path, read_result.expect_ok() });
  }

  static jtl::result<file_view, error_ref> map_file(jtl::immutable_string const &path)
  {
    if(!std::filesystem::exists(path.c_str()))
    {
      return error::runtime_unable_to_open_file(util::format("File '{}' doesn't exist.", path));
    }
    auto const file_size(std::filesystem::file_size(path.c_str()));
    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
    auto const fd(::open(path.c_str(), O_RDONLY));
    if(fd < 0)
    {
      return error::runtime_unable_to_open_file(util::format("Unable to open file '{}'.", path));
    }
    auto const head(
      reinterpret_cast<char const *>(mmap(nullptr, file_size, PROT_READ, MAP_PRIVATE, fd, 0)));

    /* MAP_FAILED is a macro which does a C-style cast. */
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast,performance-no-int-to-ptr) */
    if(head == MAP_FAILED)
#pragma clang diagnostic pop
    {
      return error::runtime_unable_to_open_file(util::format("Unable to map file '{}'.", path));
    }

    return ok(file_view{ path, fd, head, file_size });
  }

  jtl::result<file_view, error_ref> loader::read_file(jtl::immutable_string const &path)
  {
    if(path == read::no_source_path)
    {
      return error::internal_runtime_failure("No source to read.");
    }

    if(path.contains(".jar:"))
    {
      return read_jar_file(path);
    }
    return map_file(path);
  }

  jtl::result<file_view, error_ref> loader::read_module(jtl::immutable_string const &module)
  {
    auto const &found_module{ loader::find(module, origin::source) };
    if(found_module.is_err())
    {
      return found_module.expect_err();
    }

    jtl::option<file_entry> res{};

    auto const module_type_to_load{ found_module.expect_ok().to_load.unwrap() };
    auto const &module_sources{ found_module.expect_ok().sources };

    switch(module_type_to_load)
    {
      case module_type::jank:
        res = module_sources.jank.unwrap();
        break;
      case module_type::o:
        res = module_sources.o.unwrap();
        break;
      case module_type::cpp:
        res = module_sources.cpp.unwrap();
        break;
      case module_type::cljc:
        res = module_sources.cljc.unwrap();
        break;
    }

    if(res.is_none())
    {
      return error::internal_runtime_failure(util::format("Unknown type for module '{}'.", module));
    }

    auto const &entry{ res.unwrap() };
    if(entry.archive_path.is_some())
    {
      file_view file;
      auto const visit_res{ visit_jar_entry(entry,
                                            [&](zip_t * const zip) -> jtl::result<void, error_ref> {
                                              auto const read_result{ read_zip_entry(zip) };
                                              if(read_result.is_err())
                                              {
                                                return read_result.expect_err();
                                              }
                                              file = file_view{ entry.archive_path.unwrap(),
                                                                read_result.expect_ok() };
                                              return ok();
                                            }) };
      if(visit_res.is_err())
      {
        return visit_res.expect_err();
      }
      return file;
    }
    else
    {
      return module::loader::read_file(entry.path);
    }
  }

  static jtl::option<loader::entry>
  find_module(native_unordered_map<jtl::immutable_string, loader::entry> &entries,
              jtl::immutable_string const &paths,
              jtl::immutable_string const &module)
  {
    auto const &first_find(entries.find(module));

    if(first_find != entries.end())
    {
      return first_find->second;
    }

    /* Skip jars after the initial indexing since we can consider them immutable
     * for development purposes. */
    register_module_path(entries, paths, true);

    auto const &second_find(entries.find(module));

    if(second_find != entries.end())
    {
      return second_find->second;
    }

    return {};
  }

  jtl::result<loader::find_result, error_ref>
  loader::find(jtl::immutable_string const &module, origin const ori)
  {
    static std::regex const underscore{ "_" };
    native_transient_string patched_module{ module };
    patched_module = std::regex_replace(patched_module, underscore, "-");
    auto const &found(find_module(entries, paths, patched_module));

    if(found.is_none())
    {
      /* TODO: If it contains -, suggest using _. Very common issue. */
      return error::runtime_module_not_found(util::format("Unable to find module '{}'.", module));
    }

    auto const &entry(found.unwrap());

    if(ori == origin::source)
    {
      if(entry.jank.is_some())
      {
        return find_result{ entry, module_type::jank };
      }
      else if(entry.cljc.is_some())
      {
        return find_result{ entry, module_type::cljc };
      }
    }
    else
    {
      /* Ignoring object files from the archives here for security and portability
       * reasons.
       *
       * Security:
       * A dependency can include a binary version of a module that doesn't belong
       * to it.
       *
       * Portability:
       * Unlike class files, object files are tied to the OS, architecture, C++ stdlib etc,
       * making it hard to share them. */
      if(entry.o.is_some() && entry.o.unwrap().archive_path.is_none() && entry.o.unwrap().exists()
         && (entry.jank.is_some() || entry.cljc.is_some() || entry.cpp.is_some()))
      {
        auto const o_file_path{ native_transient_string{ entry.o.unwrap().path } };

        std::time_t source_modified_time{};
        module_type module_type{};

        if(entry.jank.is_some() && entry.jank.unwrap().exists())
        {
          source_modified_time = entry.jank.unwrap().last_modified_at();
          module_type = module_type::jank;
        }
        else if(entry.cljc.is_some() && entry.cljc.unwrap().exists())
        {
          source_modified_time = entry.cljc.unwrap().last_modified_at();
          module_type = module_type::cljc;
        }
        else if(entry.cpp.is_some() && entry.cpp.unwrap().exists())
        {
          source_modified_time = entry.cpp.unwrap().last_modified_at();
          module_type = module_type::cpp;
        }
        else
        {
          return error::runtime_module_binary_without_source(
            util::format("Found a binary '{}' without a source while trying to load module '{}'. "
                         "This module won't be loaded, since jank will not trust module binaries "
                         "which are missing a corresponding source.",
                         entry.o.unwrap().path,
                         module));
        }

        if(std::filesystem::last_write_time(o_file_path).time_since_epoch().count()
           >= source_modified_time)
        {
          return find_result{ entry, module_type::o };
        }
        else
        {
          return find_result{ entry, module_type };
        }
      }
      else if(entry.cpp.is_some())
      {
        return find_result{ entry, module_type::cpp };
      }
      else if(entry.jank.is_some())
      {
        return find_result{ entry, module_type::jank };
      }
      else if(entry.cljc.is_some())
      {
        return find_result{ entry, module_type::cljc };
      }
    }

    return error::internal_runtime_failure(
      util::format("No sources for registered module '{}'.", module));
  }

  bool loader::is_loaded(jtl::immutable_string const &module)
  {
    auto const atom{
      runtime::try_object<runtime::obj::atom>(__rt_ctx->loaded_libs_var->deref())->deref()
    };

    auto const loaded_libs{ runtime::try_object<runtime::obj::persistent_sorted_set>(atom) };
    auto const ret{ truthy(loaded_libs->contains(make_box<obj::symbol>(module))) };

    //util::println("is module loaded {}: {}", module, ret);

    return ret;
  }

  void loader::set_is_loaded(jtl::immutable_string const &module)
  {
    auto const loaded_libs_atom{ runtime::try_object<runtime::obj::atom>(
      __rt_ctx->loaded_libs_var->deref()) };

    auto const swap_fn{ [&](object_ref const curr_val) {
      return runtime::try_object<runtime::obj::persistent_sorted_set>(curr_val)->conj(
        make_box<obj::symbol>(module));
    } };

    auto const swap_fn_wrapper{ make_box<runtime::obj::native_function_wrapper>(
      std::function<object_ref(object_ref const)>{ swap_fn }) };
    loaded_libs_atom->swap(swap_fn_wrapper);
  }

  [[maybe_unused]]
  static void log_load(jtl::immutable_string const &module,
                       module_type const type,
                       loader::entry const &sources)
  {
    jtl::immutable_string path{ "undefined" };
    switch(type)
    {
      case module_type::jank:
        {
          auto const &source{ sources.jank.unwrap() };
          if(source.archive_path.is_some())
          {
            path = util::format("{}:{}",
                                util::relative_path(source.archive_path.unwrap()),
                                source.path);
          }
          else
          {
            path = util::relative_path(source.path);
          }
        }
        break;
      case module_type::o:
        {
          auto const &source{ sources.o.unwrap() };
          if(source.archive_path.is_some())
          {
            path = util::format("{}:{}",
                                util::relative_path(source.archive_path.unwrap()),
                                source.path);
          }
          else
          {
            path = util::relative_path(source.path);
          }
        }
        break;
      case module_type::cpp:
        {
          auto const &source{ sources.cpp.unwrap() };
          if(source.archive_path.is_some())
          {
            path = util::format("{}:{}",
                                util::relative_path(source.archive_path.unwrap()),
                                source.path);
          }
          else
          {
            path = util::relative_path(source.path);
          }
        }
        break;
      case module_type::cljc:
        {
          auto const &source{ sources.cljc.unwrap() };
          if(source.archive_path.is_some())
          {
            path = util::format("{}:{}",
                                util::relative_path(source.archive_path.unwrap()),
                                source.path);
          }
          else
          {
            path = util::relative_path(source.path);
          }
        }
        break;
    }
    util::println("Loading module {} from {}", module, path);
  }

  jtl::result<void, error_ref> loader::load(jtl::immutable_string const &module, origin const ori)
  {
    if(ori != origin::source && loader::is_loaded(module))
    {
      return ok();
    }

    auto const &found_module{ loader::find(module, ori) };
    if(found_module.is_err())
    {
      return found_module.expect_err();
    }

    jtl::result<void, error_ref> res{ ok() };

    auto const module_type_to_load{ found_module.expect_ok().to_load.unwrap() };
    auto const &module_sources{ found_module.expect_ok().sources };

    //log_load(module, module_type_to_load, module_sources);

    switch(module_type_to_load)
    {
      case module_type::jank:
        res = load_jank(module_sources.jank.unwrap());
        break;
      case module_type::o:
        res = load_o(module, module_sources.o.unwrap());
        break;
      case module_type::cpp:
        res = load_cpp(module, module_sources.cpp.unwrap());
        break;
      case module_type::cljc:
        res = load_cljc(module_sources.cljc.unwrap());
        break;
      default:
        res = error::internal_runtime_failure(
          util::format("Unknown module type '{}'.", module_type_to_load));
    }

    if(res.is_err())
    {
      return res;
    }

    loader::set_is_loaded(module);
    {
      auto const locked_ordered_modules{ __rt_ctx->loaded_modules_in_order.wlock() };
      locked_ordered_modules->push_back(module);
    }
    return ok();
  }

  jtl::result<void, error_ref>
  loader::load_o(jtl::immutable_string const &module, file_entry const &entry) const
  {
    profile::timer const timer{ util::format("load object {}", module) };

    /* While loading an object, if the main ns loading symbol exists, then
     * we don't need to load the object file again.
     *
     * Existence of the `jank_load_<module>` symbol (also a function),
     * means that all the required symbols exist and are already defined.
     * We call this symbol to re-initialize all the vars in the namespace.
     * */
    auto const load_function_name{ module_to_load_function(module) };

    auto const existing_load{ __rt_ctx->jit_prc.find_symbol(load_function_name) };
    if(existing_load.is_ok())
    {
      /* TODO: Update LLVM IR for this. */
      reinterpret_cast<void (*)()>(existing_load.expect_ok())();
      return ok();
    }

    /* We intentionally don't load object files from JARs. */
    if(entry.archive_path.is_some())
    {
    }
    else
    {
      __rt_ctx->jit_prc.load_object(entry.path);
    }

    auto const load_fn_res{ __rt_ctx->jit_prc.find_symbol(load_function_name).expect_ok() };
    reinterpret_cast<object *(*)()>(load_fn_res)();

    return ok();
  }

  jtl::result<void, error_ref>
  loader::load_cpp(jtl::immutable_string const &module, file_entry const &entry) const
  {
    if(entry.archive_path.is_some())
    {
      jtl::result<void, error_ref> res{ ok() };
      auto const visit_res{ visit_jar_entry(entry,
                                            [&](zip_t * const zip) -> jtl::result<void, error_ref> {
                                              auto const read_result{ read_zip_entry(zip) };
                                              if(read_result.is_err())
                                              {
                                                return read_result.expect_err();
                                              }
                                              res = __rt_ctx->eval_cpp_string(
                                                read_result.expect_ok());
                                              return ok();
                                            }) };
      if(res.is_err())
      {
        return res;
      }
      if(visit_res.is_err())
      {
        return visit_res;
      }
    }
    else
    {
      auto const file(module::loader::read_file(entry.path));
      if(file.is_err())
      {
        return file.expect_err();
      }
      auto const res{ __rt_ctx->eval_cpp_string(file.expect_ok().view()) };
      if(res.is_err())
      {
        return res;
      }
    }

    /* TODO: What if there is no load function?
     * What if load function is defined in another module?
     * What if load function is already loaded/defined? The llvm::Interpreter::Execute will fail. */
    auto const load_function_name{ module_to_load_function(module) };
    auto const load{ __rt_ctx->jit_prc.find_symbol(load_function_name).expect_ok() };
    reinterpret_cast<void (*)()>(load)();

    return ok();
  }

  jtl::result<void, error_ref> loader::load_jank(file_entry const &entry) const
  {
    if(entry.archive_path.is_some())
    {
      auto const res{ visit_jar_entry(
        entry,
        [&](zip_t * const zip) -> jtl::result<void, error_ref> {
          auto const read_result{ read_zip_entry(zip) };
          if(read_result.is_err())
          {
            return read_result.expect_err();
          }

          /* TODO: Helper to get a jar file path like this. */
          auto const path{ util::format("{}:{}", entry.archive_path.unwrap(), entry.path) };
          context::binding_scope const preserve{ runtime::obj::persistent_hash_map::create_unique(
            std::make_pair(__rt_ctx->current_file_var, make_box(path))) };
          __rt_ctx->eval_string(read_result.expect_ok());
          return ok();
        }) };
      if(res.is_err())
      {
        return res;
      }
    }
    else
    {
      __rt_ctx->eval_file(entry.path);
    }

    return ok();
  }

  jtl::result<void, error_ref> loader::load_cljc(file_entry const &entry) const
  {
    return load_jank(entry);
  }

  void loader::add_path(jtl::immutable_string const &path)
  {
    jtl::string_builder sb;
    sb(paths);
    sb(module_separator);
    sb(path);
    paths = sb.release();
    register_path(entries, path);
  }

  object_ref loader::to_runtime_data() const
  {
    runtime::object_ref entry_maps(make_box<runtime::obj::persistent_array_map>());
    for(auto const &e : entries)
    {
      entry_maps = runtime::assoc(entry_maps,
                                  make_box(e.first),
                                  runtime::obj::persistent_array_map::create_unique(
                                    make_box("jank"),
                                    jank::detail::to_runtime_data(e.second.jank),
                                    make_box("cljc"),
                                    jank::detail::to_runtime_data(e.second.cljc),
                                    make_box("cpp"),
                                    jank::detail::to_runtime_data(e.second.cpp),
                                    make_box("o"),
                                    jank::detail::to_runtime_data(e.second.o)));
    }

    return runtime::obj::persistent_array_map::create_unique(make_box("__type"),
                                                             make_box("module::loader"),
                                                             make_box("entries"),
                                                             entry_maps);
  }
}
