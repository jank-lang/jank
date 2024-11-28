#include <regex>
#include <iostream>

#include <libzippp.h>

#include <boost/filesystem.hpp>

#include <jank/util/mapped_file.hpp>
#include <jank/util/process_location.hpp>
#include <jank/runtime/module/loader.hpp>
#include <jank/runtime/core.hpp>
#include <jank/runtime/core/munge.hpp>
#include <jank/runtime/context.hpp>
#include <jank/profile/time.hpp>

namespace jank::runtime::module
{
  /* This turns `foo_bar/spam/meow.cljc` into `foo-bar.spam.meow`. */
  native_persistent_string path_to_module(boost::filesystem::path const &path)
  {
    static std::regex const slash{ "/" };

    auto const &s(runtime::demunge(path.string()));
    std::string ret{ s, 0, s.size() - path.extension().size() };

    /* There's a special case of the / function which shouldn't be treated as a path. */
    if(ret.find("$/") == std::string::npos)
    {
      ret = std::regex_replace(ret, slash, ".");
    }

    return ret;
  }

  native_persistent_string module_to_path(native_persistent_string_view const &module)
  {
    static std::regex const dot{ "\\." };

    std::string ret{ runtime::munge(module) };
    ret = std::regex_replace(ret, dot, "/");

    return ret;
  }

  native_persistent_string module_to_load_function(native_persistent_string_view const &module)
  {
    static std::regex const dot{ "\\." };

    std::string ret{ runtime::munge(module) };
    ret = std::regex_replace(ret, dot, "_");

    return fmt::format("jank_load_{}", ret);
  }

  /* This is a somewhat complicated function. We take in a module (doesn't need to be munged) and
   * we return a native namespace name. So foo.bar will become foo::bar. But we also strip off
   * the last nested module, since the way the codegen works is that foo.bar$spam lives in the
   * native namespace foo::bar. Lastly, we need to split the module into parts and munge each
   * individually, since we can have a module like clojure.template which will munge cleanly
   * on its own, but template is a C++ keyword and the resulting clojure::template namespace
   * will be a problem. So we split the module on each ., munge, and put it back together
   * using ::. */
  native_persistent_string module_to_native_ns(native_persistent_string_view const &orig_module)
  {
    static std::regex const dollar{ "\\$" };

    native_transient_string module{ munge(orig_module) };

    native_vector<native_transient_string> module_parts;
    for(size_t dot_pos{}; (dot_pos = module.find('.')) != native_persistent_string::npos;)
    {
      module_parts.emplace_back(munge(module.substr(0, dot_pos)));
      module.erase(0, dot_pos + 1);
    }

    if(module.find('$') != native_transient_string::npos)
    {
      for(size_t dollar_pos{}; (dollar_pos = module.find('$')) != native_persistent_string::npos;)
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

  native_persistent_string
  nest_module(native_persistent_string const &module, native_persistent_string const &sub)
  {
    assert(!module.empty());
    assert(!sub.empty());
    return module + "$" + sub;
  }

  native_persistent_string
  nest_native_ns(native_persistent_string const &native_ns, native_persistent_string const &end)
  {
    assert(!native_ns.empty());
    assert(!end.empty());
    return fmt::format("::{}::{}", native_ns, end);
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
      throw std::runtime_error{ fmt::format("Failed to open jar on classpath: {}", path) };
    }

    auto const &zip_entry(zf.getEntry(std::string{ entry.path }));
    fn(zip_entry.readAsText());
  }

  static void register_entry(native_unordered_map<native_persistent_string, loader::entry> &entries,
                             boost::filesystem::path const &resource_path,
                             file_entry const &entry)
  {
    boost::filesystem::path const p{ native_transient_string{ entry.path } };
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
      //fmt::println("register_entry {} {} {} {}",
      //             entry.archive_path,
      //             entry.path,
      //             module_path.string(),
      //             path_to_module(module_path));
    }
  }

  static void
  register_directory(native_unordered_map<native_persistent_string, loader::entry> &entries,
                     boost::filesystem::path const &path)
  {
    for(auto const &f : boost::filesystem::recursive_directory_iterator{ path })
    {
      if(boost::filesystem::is_regular_file(f))
      {
        register_entry(entries, path, file_entry{ none, f.path().string() });
      }
    }
  }

  static void register_jar(native_unordered_map<native_persistent_string, loader::entry> &entries,
                           native_persistent_string_view const &path)
  {
    libzippp::ZipArchive zf{ std::string{ path } };
    auto success(zf.open(libzippp::ZipArchive::ReadOnly));
    if(!success)
    {
      std::cerr << fmt::format("Failed to open jar on classpath: {}\n", path);
      return;
    }

    auto const &zip_entries(zf.getEntries());
    for(auto const &entry : zip_entries)
    {
      auto const &name(entry.getName());
      if(!entry.isDirectory())
      {
        register_entry(entries, "", { path, name });
      }
    }
  }

  static void register_path(native_unordered_map<native_persistent_string, loader::entry> &entries,
                            native_persistent_string_view const &path)
  {
    /* It's entirely possible to have empty entries in the classpath, mainly due to lazy string
     * concatenation. We just ignore them. This means something like "::::" is valid. */
    if(path.empty() || !boost::filesystem::exists(path))
    {
      return;
    }

    boost::filesystem::path const p{ boost::filesystem::canonical(path).lexically_normal() };
    if(boost::filesystem::is_directory(p))
    {
      register_directory(entries, p);
    }
    else if(p.extension().string() == ".jar")
    {
      register_jar(entries, path);
    }
    /* If it's not a JAR or a directory, we just add it as a direct file entry. I don't think the
     * JVM supports this, but I like that it allows us to put specific files in the path. */
    else
    {
      register_entry(entries, "", { none, p.string() });
    }
  }

  loader::loader(context &rt_ctx, native_persistent_string_view const &ps)
    : rt_ctx{ rt_ctx }
  {
    auto const jank_path(jank::util::process_location().unwrap().parent_path());
    native_transient_string paths{ ps };
    paths += fmt::format(":{}", (jank_path / "classes").string());
    paths += fmt::format(":{}", (jank_path / "../src/jank").string());
    paths += fmt::format(":{}", rt_ctx.output_dir);
    this->paths = paths;

    //fmt::println("classpaths: {}", paths);

    size_t start{};
    size_t i{ paths.find(module_separator, start) };

    /* Looks like it's either an empty path list or there's only entry. */
    if(i == native_persistent_string_view::npos)
    {
      register_path(entries, paths);
    }
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
    return runtime::obj::persistent_array_map::create_unique(
      make_box("__type"),
      make_box("module::file_entry"),
      make_box("archive_path"),
      jank::detail::to_runtime_data(archive_path),
      make_box("path"),
      make_box(path));
  }

  native_bool loader::is_loaded(native_persistent_string_view const &module) const
  {
    return loaded.contains(module);
  }

  void loader::set_loaded(native_persistent_string_view const &module)
  {
    loaded.emplace(module);
  }

  result<void, native_persistent_string>
  loader::load_ns(native_persistent_string_view const &module)
  {
    profile::timer const timer{ "load_ns" };

    //fmt::println("loading ns {}", module);
    auto res(load(module));
    if(res.is_err())
    {
      return res;
    }

    return ok();
  }

  result<void, native_persistent_string> loader::load(native_persistent_string_view const &module)
  {
    static std::regex const underscore{ "_" };
    native_transient_string patched_module{ module };
    patched_module = std::regex_replace(patched_module, underscore, "-");
    auto const &entry(entries.find(patched_module));
    if(entry == entries.end())
    {
      return err(fmt::format("unable to find module: {}", module));
    }

    result<void, native_persistent_string> res{ err(
      fmt::format("no sources for registered module: {}", module)) };

    //fmt::println("loading nested module {}", module);

    bool const compiling{ truthy(rt_ctx.compile_files_var->deref()) };
    if(compiling)
    {
      if(entry->second.jank.is_some())
      {
        res = load_jank(entry->second.jank.unwrap());
      }
      else if(entry->second.cljc.is_some())
      {
        res = load_cljc(entry->second.cljc.unwrap());
      }
    }
    else
    {
      if(entry->second.o.is_some())
      {
        res = load_o(module, entry->second.o.unwrap());
      }
      else if(entry->second.cpp.is_some())
      {
        res = load_cpp(entry->second.cpp.unwrap());
      }
      else if(entry->second.jank.is_some())
      {
        res = load_jank(entry->second.jank.unwrap());
      }
      else if(entry->second.cljc.is_some())
      {
        res = load_cljc(entry->second.cljc.unwrap());
      }
    }

    if(res.is_err())
    {
      return res;
    }

    loaded.emplace(module);

    return ok();
  }

  result<void, native_persistent_string>
  loader::load_o(native_persistent_string const &module, file_entry const &entry) const
  {
    profile::timer const timer{ fmt::format("load object {}", module) };
    if(entry.archive_path.is_some())
    {
      /* TODO: Load object code from string. */
      //visit_jar_entry(entry, [&](auto const &str) { rt_ctx.jit_prc.load_object(module, str); });
    }
    else
    {
      rt_ctx.jit_prc.load_object(entry.path);
    }

    auto const load{ rt_ctx.jit_prc.find_symbol<object *(*)()>(module_to_load_function(module)) };
    load();

    return ok();
  }

  result<void, native_persistent_string> loader::load_cpp(file_entry const &entry) const
  {
    if(entry.archive_path.is_some())
    {
      visit_jar_entry(entry, [&](auto const &str) { rt_ctx.jit_prc.eval_string(str); });
    }
    else
    {
      auto const file(util::map_file(entry.path));
      if(file.is_err())
      {
        return err(
          fmt::format("unable to map file {} due to error: {}", entry.path, file.expect_err()));
      }
      rt_ctx.jit_prc.eval_string({ file.expect_ok().head, file.expect_ok().size });
    }

    return ok();
  }

  result<void, native_persistent_string> loader::load_jank(file_entry const &entry) const
  {
    if(entry.archive_path.is_some())
    {
      visit_jar_entry(entry, [&](auto const &str) { rt_ctx.eval_string(str); });
    }
    else
    {
      rt_ctx.eval_file(entry.path);
    }

    return ok();
  }

  result<void, native_persistent_string> loader::load_cljc(file_entry const &) const
  {
    return err("Not yet implemented: CLJC loading");
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
