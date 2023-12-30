#include <libzippp.h>

#include <regex>

#include <boost/filesystem.hpp>

#include <jank/util/mapped_file.hpp>
#include <jank/util/process_location.hpp>
#include <jank/runtime/module/loader.hpp>

namespace jank::runtime::module
{
  /* This turns `foo_bar/spam/meow.cljc` into `foo-bar.spam.meow`. */
  native_persistent_string path_to_module(boost::filesystem::path const &path)
  {
    //static std::regex const underscore{ "_" };
    static std::regex const slash{ "/" };

    auto const &s(path.string());
    std::string ret{ s, 0, s.size() - path.extension().size() };
    //ret = std::regex_replace(ret, underscore, "-");
    ret = std::regex_replace(ret, slash, ".");

    return ret;
  }

  native_persistent_string module_to_path(native_persistent_string_view const &module)
  {
    static std::regex const dash{ "-" };
    static std::regex const dot{ "\\." };

    std::string ret{ module };
    ret = std::regex_replace(ret, dash, "_");
    ret = std::regex_replace(ret, dot, "/");

    return ret;
  }

  native_persistent_string module_to_native_ns(native_persistent_string_view const &module)
  {
    static std::regex const dash{ "-" };
    static std::regex const dot{ "\\." };
    static std::regex const last_module{ "\\$[a-zA-Z_0-9]+$" };
    static std::regex const dollar{ "\\$" };

    std::string ret{ module };
    ret = std::regex_replace(ret, dash, "_");
    ret = std::regex_replace(ret, dot, "::");
    ret = std::regex_replace(ret, last_module, "");
    ret = std::regex_replace(ret, dollar, "::");

    return ret;
  }

  native_persistent_string nest_module(native_persistent_string const &module, native_persistent_string const &sub)
  {
    assert(!module.empty());
    assert(!sub.empty());
    return module + "$" + sub;
  }

  native_persistent_string nest_native_ns(native_persistent_string const &native_ns, native_persistent_string const &end)
  {
    assert(!native_ns.empty());
    assert(!end.empty());
    return fmt::format("::{}::{}", native_ns, end);
  }

  /* If it has two or more occurences of $, it's nested. */
  native_bool is_nested_module(native_persistent_string const &module)
  { return module.find('$') != module.rfind('$'); }

  template <typename F>
  void visit_jar_entry(file_entry const &entry, F const &fn)
  {
    auto const &path(entry.archive_path.unwrap());
    libzippp::ZipArchive zf{ std::string{ path } };
    auto success(zf.open(libzippp::ZipArchive::ReadOnly));
    if(!success)
    { throw std::runtime_error{ fmt::format("Failed to open jar on classpath: {}", path) }; }

    auto const &zip_entry(zf.getEntry(std::string{ entry.path }));
    fn(zip_entry.readAsText());
  }

  void register_entry
  (
    native_unordered_map<native_persistent_string, loader::entry> &entries,
    boost::filesystem::path const &resource_path,
    file_entry const &entry
  )
  {
    boost::filesystem::path p{ entry.path };
    /* We need the file path relative to the class path, since the class
     * path portion is not included in part of the module name. For example,
     * the file may live in `src/jank/clojure/core.jank` but the module
     * should be `clojure.core`, not `src.jank.clojure.core`. */
    auto const &module_path(p.lexically_relative(resource_path));

    auto const ext(p.extension().string());
    bool registered{};
    if(ext == ".jank")
    {
      registered = true;
      loader::entry e;
      e.jank = entry;
      auto res(entries.insert({ path_to_module(module_path), std::move(e) }));
      if(!res.second)
      { res.first->second.jank = entry; }
    }
    else if(ext == ".cljc")
    {
      registered = true;
      loader::entry e;
      e.cljc = entry;
      auto res(entries.insert({ path_to_module(module_path), std::move(e) }));
      if(!res.second)
      { res.first->second.cljc = entry; }
    }
    else if(ext == ".cpp")
    {
      registered = true;
      loader::entry e;
      e.cpp = entry;
      auto res(entries.insert({ path_to_module(module_path), std::move(e) }));
      if(!res.second)
      { res.first->second.cpp = entry; }
    }
    else if(ext == ".pcm")
    {
      registered = true;
      loader::entry e;
      e.pcm = entry;
      auto res(entries.insert({ path_to_module(module_path), std::move(e) }));
      if(!res.second)
      { res.first->second.pcm = entry; }
    }

    if(registered)
    {
      //fmt::println("register_entry {} {} {} {}", entry.archive_path, entry.path, module_path.string(), path_to_module(module_path));
    }
  }

  void register_directory(native_unordered_map<native_persistent_string, loader::entry> &entries, boost::filesystem::path const &path)
  {
    for(auto const &f : boost::filesystem::recursive_directory_iterator{ path })
    {
      if(boost::filesystem::is_regular_file(f))
      { register_entry(entries, path, file_entry{ none, f.path().string() }); }
    }
  }

  void register_jar(native_unordered_map<native_persistent_string, loader::entry> &entries, native_persistent_string_view const &path)
  {
    libzippp::ZipArchive zf{ std::string{ path } };
    auto success(zf.open(libzippp::ZipArchive::ReadOnly));
    if(!success)
    {
      std::cerr << fmt::format("Failed to open jar on classpath: {}", path) << std::endl;
      return;
    }

    auto const &zip_entries(zf.getEntries());
    for(auto const &entry : zip_entries)
    {
      auto const &name(entry.getName());
      if(!entry.isDirectory())
      { register_entry(entries, "", { path, name }); }
    }
  }

  void register_path(native_unordered_map<native_persistent_string, loader::entry> &entries, native_persistent_string_view const &path)
  {
    /* It's entirely possible to have empty entries in the classpath, mainly due to lazy string
     * concatenation. We just ignore them. This means something like "::::" is valid. */
    if(path.empty() || !boost::filesystem::exists(path))
    { return; }

    boost::filesystem::path p{ boost::filesystem::canonical(path).lexically_normal() };
    if(boost::filesystem::is_directory(p))
    { register_directory(entries, p); }
    else if(p.extension().string() == ".jar")
    { register_jar(entries, path); }
    /* If it's not a JAR or a directory, we just add it as a direct file entry. I don't think the
     * JVM supports this, but I like that it allows us to put specific files in the path. */
    else
    { register_entry(entries, "", { none, p.string() }); }
  }

  loader::loader(context &rt_ctx, native_persistent_string_view const &ps)
    : rt_ctx{ rt_ctx }
  {
    auto const jank_path(jank::util::process_location().unwrap().parent_path());
    native_transient_string paths{ ps };
    paths += fmt::format(":{}", (jank_path / "../src/jank").string());
    paths += fmt::format(":{}", rt_ctx.output_dir);
    this->paths = paths;

    size_t start{};
    size_t i{ paths.find(module_separator, start) };

    /* Looks like it's either an empty path list or there's only entry. */
    if(i == native_persistent_string_view::npos)
    { register_path(entries, paths); }
    else
    {
      while(i != native_persistent_string_view::npos)
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
    return runtime::obj::persistent_array_map::create_unique
    (
      make_box("__type"), make_box("module::file_entry"),
      make_box("archive_path"), jank::detail::to_runtime_data(archive_path),
      make_box("path"), make_box(path)
    );
  }

  native_bool loader::is_loaded(native_persistent_string_view const &module) const
  { return loaded.contains(module); }

  void loader::set_loaded(native_persistent_string_view const &module)
  { loaded.emplace(module); }

  result<void, native_persistent_string> loader::load_ns(native_persistent_string_view const &module)
  {
    profile::timer timer{ "load_ns" };
    bool const compiling{ rt_ctx.compiling };
    native_bool const needs_init{ !compiling && entries.contains(fmt::format("{}__init", module)) };
    if(needs_init)
    {
      profile::timer timer{ "load_ns __init" };
      auto ret(load(fmt::format("{}__init", module)));
      if(ret.is_err())
      { return ret; }
      rt_ctx.jit_prc.eval_string
      (
        fmt::format
        (
          "{}::__ns__init::__init();",
          runtime::module::module_to_native_ns(module)
        )
      );
    }

    {
      profile::timer timer{ "load_ns compilation" };
      auto res(load(module));
      if(res.is_err())
      { return res; }
    }

    if(needs_init)
    {
      profile::timer timer{ "load_ns effects" };
      rt_ctx.jit_prc.eval_string
      (
        fmt::format
        (
          "{}::__ns{{ __rt_ctx }}.call();",
          runtime::module::module_to_native_ns(module)
        )
      );
    }

    return ok();
  }

  result<void, native_persistent_string> loader::load(native_persistent_string_view const &module)
  {
    auto const &entry(entries.find(module));
    if(entry == entries.end())
    { return err(fmt::format("unable to find module: {}", module)); }

    result<void, native_persistent_string> res
    { err(fmt::format("no sources for registered module: {}", module)) };

    bool const compiling{ rt_ctx.compiling };
    if(compiling)
    {
      if(entry->second.jank.is_some())
      { res = load_jank(entry->second.jank.unwrap()); }
      else if(entry->second.cljc.is_some())
      { res = load_cljc(entry->second.cljc.unwrap()); }
    }
    else
    {
      if(entry->second.pcm.is_some())
      { res = load_pcm(entry->second.pcm.unwrap()); }
      else if(entry->second.cpp.is_some())
      { res = load_cpp(entry->second.cpp.unwrap()); }
      else if(entry->second.jank.is_some())
      { res = load_jank(entry->second.jank.unwrap()); }
      else if(entry->second.cljc.is_some())
      { res = load_cljc(entry->second.cljc.unwrap()); }
    }

    if(res.is_err())
    { return res; }

    loaded.emplace(module);

    return ok();
  }

  result<void, native_persistent_string> loader::load_pcm(file_entry const &) const
  { return err("Not yet implemented: PCM loading"); }

  result<void, native_persistent_string> loader::load_cpp(file_entry const &entry) const
  {
    if(entry.archive_path.is_some())
    {
      visit_jar_entry
      (
        entry,
        [&](auto const &str)
        { rt_ctx.jit_prc.eval_string(str); }
      );
    }
    else
    {
      auto const file(util::map_file(entry.path));
      if(file.is_err())
      { return err(fmt::format("unable to map file {} due to error: {}", entry.path, file.expect_err())); }
      rt_ctx.jit_prc.eval_string({ file.expect_ok().head, file.expect_ok().size });
    }

    return ok();
  }

  result<void, native_persistent_string> loader::load_jank(file_entry const &entry) const
  {
    if(entry.archive_path.is_some())
    {
      visit_jar_entry
      (
        entry,
        [&](auto const &str)
        { rt_ctx.eval_string(str); }
      );
    }
    else
    { rt_ctx.eval_file(entry.path); }

    return ok();
  }

  result<void, native_persistent_string> loader::load_cljc(file_entry const &) const
  { return err("Not yet implemented: CLJC loading"); }

  object_ptr loader::to_runtime_data() const
  {
    runtime::object_ptr entry_maps(make_box<runtime::obj::persistent_array_map>());
    for(auto const &e : entries)
    {
      entry_maps = runtime::assoc
      (
        entry_maps,
        make_box(e.first),
        runtime::obj::persistent_array_map::create_unique
        (
          make_box("jank"), jank::detail::to_runtime_data(e.second.jank),
          make_box("cljc"), jank::detail::to_runtime_data(e.second.cljc),
          make_box("cpp"), jank::detail::to_runtime_data(e.second.cpp),
          make_box("pcm"), jank::detail::to_runtime_data(e.second.pcm)
        )
      );
    }

    return runtime::obj::persistent_array_map::create_unique
    (
      make_box("__type"), make_box("module::loader"),
      make_box("entries"), entry_maps
    );
  }
}
