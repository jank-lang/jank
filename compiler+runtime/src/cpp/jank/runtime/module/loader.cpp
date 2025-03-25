#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include <filesystem>
#include <regex>

#include <libzippp.h>

#include <jank/util/process_location.hpp>
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
#include <jank/profile/time.hpp>

namespace jank::runtime::module
{
  /* This turns `foo_bar/spam/meow.cljc` into `foo-bar.spam.meow`. */
  native_persistent_string path_to_module(std::filesystem::path const &path)
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

  native_persistent_string module_to_path(native_persistent_string const &module)
  {
    static native_persistent_string const dot{ "\\." };
    return runtime::munge_extra(module, dot, "/");
  }

  native_persistent_string module_to_load_function(native_persistent_string const &module)
  {
    static native_persistent_string const dot{ "\\." };
    auto const &ret{ runtime::munge_extra(module, dot, "_") };

    return util::format("jank_load_{}", ret);
  }

  native_persistent_string
  nest_module(native_persistent_string const &module, native_persistent_string const &sub)
  {
    jank_debug_assert(!module.empty());
    jank_debug_assert(!sub.empty());
    return module + "$" + sub;
  }

  native_persistent_string
  nest_native_ns(native_persistent_string const &native_ns, native_persistent_string const &end)
  {
    jank_debug_assert(!native_ns.empty());
    jank_debug_assert(!end.empty());
    return util::format("::{}::{}", native_ns, end);
  }

  /* If it has two or more occurences of $, it's nested. */
  native_bool is_nested_module(native_persistent_string const &module)
  {
    return module.find('$') != module.rfind('$');
  }

  /* TODO: We can patch libzippp to not copy strings around so much. */
  template <typename F>
  static void visit_jar_entry(file_entry const &entry, F const &fn)
  {
    auto const &path(entry.archive_path.unwrap());
    libzippp::ZipArchive zf{ std::string{ path } };
    auto const success(zf.open(libzippp::ZipArchive::ReadOnly));
    if(!success)
    {
      throw std::runtime_error{ util::format("Failed to open jar on module path: {}", path) };
    }

    auto const &zip_entry(zf.getEntry(std::string{ entry.path }));
    fn(zip_entry);
  }

  static void register_entry(native_unordered_map<native_persistent_string, loader::entry> &entries,
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
  register_relative_entry(native_unordered_map<native_persistent_string, loader::entry> &entries,
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
  register_directory(native_unordered_map<native_persistent_string, loader::entry> &entries,
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

  static void register_jar(native_unordered_map<native_persistent_string, loader::entry> &entries,
                           native_persistent_string const &path)
  {
    libzippp::ZipArchive zf{ std::string{ path } };
    auto success(zf.open(libzippp::ZipArchive::ReadOnly));
    if(!success)
    {
      //util::println(stderr, "Failed to open jar on module path: {}\n", path);
      return;
    }

    auto const &zip_entries(zf.getEntries());
    for(auto const &entry : zip_entries)
    {
      auto const &name(entry.getName());
      if(!entry.isDirectory())
      {
        register_entry(entries, name, { path, name });
      }
    }
  }

  static void register_path(native_unordered_map<native_persistent_string, loader::entry> &entries,
                            native_persistent_string_view const &path)
  {
    /* It's entirely possible to have empty entries in the module path, mainly due to lazy string
     * concatenation. We just ignore them. This means something like "::::" is valid. */
    if(path.empty() || !std::filesystem::exists(path))
    {
      return;
    }

    std::filesystem::path const p{ std::filesystem::canonical(path).lexically_normal() };
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

  loader::loader(context &rt_ctx, native_persistent_string const &ps)
    : rt_ctx{ rt_ctx }
  {
    auto const jank_path(jank::util::process_location().unwrap().parent_path());
    native_transient_string paths{ ps };
    paths += util::format(":{}", rt_ctx.binary_cache_dir);
    paths += util::format(":{}", (jank_path / rt_ctx.binary_cache_dir.c_str()).native());
    paths += util::format(":{}", (jank_path / "../src/jank").native());
    this->paths = paths;

    //util::println("module paths: {}", paths);

    size_t start{};
    size_t i{ paths.find(module_separator, start) };

    /* Looks like it's either an empty path list or there's only entry. */
    if(i == native_persistent_string::npos)
    {
      register_path(entries, paths);
    }
    else
    {
      while(i != native_persistent_string::npos)
      {
        register_path(entries, paths.substr(start, i - start));

        start = i + 1;
        i = paths.find(module_separator, start);
      }

      register_path(entries, paths.substr(start, i - start));
    }
  }

  object_ptr file_entry::to_runtime_data() const
  {
    return runtime::obj::persistent_array_map::create_unique(
      make_box("__type"),
      make_box("module::file_entry"),
      make_box("archive_path"),
      jank::detail::to_runtime_data(archive_path),
      make_box("path"),
      make_box(path));
  }

  native_bool file_entry::exists() const
  {
    auto const is_archive{ archive_path.is_some() };
    if(is_archive && !std::filesystem::exists(native_transient_string{ archive_path.unwrap() }))
    {
      return false;
    }
    else
    {
      native_bool source_exists{};
      if(is_archive)
      {
        visit_jar_entry(*this, [&](auto const &zip_entry) { source_exists = zip_entry.isFile(); });
      }

      return source_exists || std::filesystem::exists(native_transient_string{ path });
    }
  }

  std::time_t file_entry::last_modified_at() const
  {
    auto const source_path{ archive_path.unwrap_or(path) };
    return std::filesystem::last_write_time(native_transient_string{ source_path })
      .time_since_epoch()
      .count();
  }

  file_view::file_view(file_view &&mf) noexcept
    : fd{ mf.fd }
    , head{ mf.head }
    , len{ mf.len }
    , buff{ std::move(mf.buff) }
  {
    mf.fd = -1;
    mf.head = nullptr;
  }

  file_view::file_view(int const f, char const * const h, size_t const s)
    : fd{ f }
    , head{ h }
    , len{ s }
  {
  }

  file_view::file_view(native_persistent_string const &buff)
    : buff{ buff }
  {
  }

  file_view::~file_view()
  {
    if(head != nullptr)
    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast): I want const everywhere else. */
    {
      munmap(reinterpret_cast<void *>(const_cast<char *>(head)), len);
    }
    if(fd >= 0)
    {
      ::close(fd);
    }
  }

  char const *file_view::data() const
  {
    return buff.empty() ? head : buff.data();
  }

  size_t file_view::size() const
  {
    return buff.empty() ? len : buff.size();
  }

  native_persistent_string_view file_view::view() const
  {
    return { data(), size() };
  }

  static string_result<file_view> read_jar_file(native_persistent_string const &path)
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

    libzippp::ZipArchive zf{ std::string{ jar_path } };
    auto const success{ zf.open(libzippp::ZipArchive::ReadOnly) };
    if(!success)
    {
      return err(util::format("Failed to open jar on module path: {}", path));
    }

    auto const &zip_entry{ zf.getEntry(std::string{ file_path }) };
    return ok(file_view{ zip_entry.readAsText() });
  }

  static string_result<file_view> map_file(native_persistent_string const &path)
  {
    if(!std::filesystem::exists(path.c_str()))
    {
      return err("File doesn't exist");
    }
    auto const file_size(std::filesystem::file_size(path.c_str()));
    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
    auto const fd(::open(path.c_str(), O_RDONLY));
    if(fd < 0)
    {
      return err("Unable to open file");
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
      return err("Mapping failed for unknown reason");
    }

    return ok(file_view{ fd, head, file_size });
  }

  string_result<file_view> loader::read_file(native_persistent_string const &path)
  {
    if(path.contains(".jar:"))
    {
      return read_jar_file(path);
    }
    return map_file(path);
  }

  string_result<loader::find_result>
  loader::find(native_persistent_string const &module, origin const ori)
  {
    static std::regex const underscore{ "_" };
    native_transient_string patched_module{ module };
    patched_module = std::regex_replace(patched_module, underscore, "-");
    auto const &entry(entries.find(patched_module));
    if(entry == entries.end())
    {
      return err(util::format("unable to find module: {}", module));
    }

    if(ori == origin::source)
    {
      if(entry->second.jank.is_some())
      {
        return find_result{ entry->second, module_type::jank };
      }
      else if(entry->second.cljc.is_some())
      {
        return find_result{ entry->second, module_type::cljc };
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
      if(entry->second.o.is_some() && entry->second.o.unwrap().archive_path.is_none()
         && entry->second.o.unwrap().exists()
         && (entry->second.jank.is_some() || entry->second.cljc.is_some()
             || entry->second.cpp.is_some()))
      {
        auto const o_file_path{ native_transient_string{ entry->second.o.unwrap().path } };

        std::time_t source_modified_time{};
        module_type module_type{};

        if(entry->second.jank.is_some() && entry->second.jank.unwrap().exists())
        {
          source_modified_time = entry->second.jank.unwrap().last_modified_at();
          module_type = module_type::jank;
        }
        else if(entry->second.cljc.is_some() && entry->second.cljc.unwrap().exists())
        {
          source_modified_time = entry->second.cljc.unwrap().last_modified_at();
          module_type = module_type::cljc;
        }
        else if(entry->second.cpp.is_some() && entry->second.cpp.unwrap().exists())
        {
          source_modified_time = entry->second.cpp.unwrap().last_modified_at();
          module_type = module_type::cpp;
        }
        else
        {
          return err(
            util::format("Found a binary ({}), without a source", entry->second.o.unwrap().path));
        }

        if(std::filesystem::last_write_time(o_file_path).time_since_epoch().count()
           >= source_modified_time)
        {
          return find_result{ entry->second, module_type::o };
        }
        else
        {
          return find_result{ entry->second, module_type };
        }
      }
      else if(entry->second.cpp.is_some())
      {
        return find_result{ entry->second, module_type::cpp };
      }
      else if(entry->second.jank.is_some())
      {
        return find_result{ entry->second, module_type::jank };
      }
      else if(entry->second.cljc.is_some())
      {
        return find_result{ entry->second, module_type::cljc };
      }
    }

    return err(util::format("No sources for registered module: {}", module));
  }

  native_bool loader::is_loaded(native_persistent_string const &module)
  {
    auto const atom{
      runtime::try_object<runtime::obj::atom>(__rt_ctx->loaded_libs_var->deref())->deref()
    };

    auto const loaded_libs{ runtime::try_object<runtime::obj::persistent_sorted_set>(atom) };
    return truthy(loaded_libs->contains(make_box<obj::symbol>(module)));
  }

  void loader::set_is_loaded(native_persistent_string const &module)
  {
    auto const loaded_libs_atom{ runtime::try_object<runtime::obj::atom>(
      __rt_ctx->loaded_libs_var->deref()) };

    auto const swap_fn{ [&](object_ptr const curr_val) {
      return runtime::try_object<runtime::obj::persistent_sorted_set>(curr_val)->conj(
        make_box<obj::symbol>(module));
    } };

    auto const swap_fn_wrapper{ make_box<runtime::obj::native_function_wrapper>(
      std::function<object_ptr(object_ptr)>{ swap_fn }) };
    loaded_libs_atom->swap(swap_fn_wrapper);
  }

  [[maybe_unused]]
  static void log_load(native_persistent_string const &module,
                       module_type const type,
                       loader::entry const &sources)
  {
    native_persistent_string path{ "undefined" };
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

  string_result<void> loader::load(native_persistent_string const &module, origin const ori)
  {
    if(ori != origin::source && loader::is_loaded(module))
    {
      return ok();
    }

    auto const &found_module{ loader::find(module, ori) };
    if(found_module.is_err())
    {
      return err(found_module.expect_err());
    }

    string_result<void> res(err(util::format("Couldn't load module: {}", module)));

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
    }

    if(res.is_err())
    {
      return res;
    }

    loader::set_is_loaded(module);
    return ok();
  }

  string_result<void>
  loader::load_o(native_persistent_string const &module, file_entry const &entry) const
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

    auto const existing_load{ rt_ctx.jit_prc.find_symbol<object *(*)()>(load_function_name) };
    if(existing_load.is_ok())
    {
      existing_load.expect_ok()();
      return ok();
    }

    /* We intentionally don't load object files from JARs. */
    if(entry.archive_path.is_some())
    {
    }
    else
    {
      rt_ctx.jit_prc.load_object(entry.path);
    }

    auto const load{ rt_ctx.jit_prc.find_symbol<object *(*)()>(load_function_name).expect_ok() };
    load();

    return ok();
  }

  string_result<void>
  loader::load_cpp(native_persistent_string const &module, file_entry const &entry) const
  {
    if(entry.archive_path.is_some())
    {
      visit_jar_entry(entry, [&](auto const &zip_entry) {
        rt_ctx.eval_cpp_string(zip_entry.readAsText());
      });
    }
    else
    {
      auto const file(module::loader::read_file(entry.path));
      if(file.is_err())
      {
        return err(
          util::format("Unable to map file {} due to error: {}", entry.path, file.expect_err()));
      }
      rt_ctx.eval_cpp_string(file.expect_ok().view());
    }

    /* TODO: What if there is no load function?
     * What if load function is defined in another module?
     * What if load function is already loaded/defined? The llvm::Interpreter::Execute will fail. */
    auto const load_function_name{ module_to_load_function(module) };
    auto const load{ rt_ctx.jit_prc.find_symbol<object *(*)()>(load_function_name).expect_ok() };
    load();

    return ok();
  }

  string_result<void> loader::load_jank(file_entry const &entry) const
  {
    if(entry.archive_path.is_some())
    {
      visit_jar_entry(entry, [&](auto const &zip_entry) {
        /* TODO: Helper to get a jar file path like this. */
        auto const path{ util::format("{}:{}", entry.archive_path.unwrap(), entry.path) };
        context::binding_scope const preserve{ rt_ctx,
                                               runtime::obj::persistent_hash_map::create_unique(
                                                 std::make_pair(rt_ctx.current_file_var,
                                                                make_box(path))) };
        rt_ctx.eval_string(zip_entry.readAsText());
      });
    }
    else
    {
      rt_ctx.eval_file(entry.path);
    }

    return ok();
  }

  string_result<void> loader::load_cljc(file_entry const &entry) const
  {
    return load_jank(entry);
  }

  object_ptr loader::to_runtime_data() const
  {
    runtime::object_ptr entry_maps(make_box<runtime::obj::persistent_array_map>());
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
