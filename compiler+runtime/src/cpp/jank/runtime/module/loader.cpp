#include <boost/filesystem/operations.hpp>
#include <regex>
#include <iostream>

#include <libzippp.h>

#include <boost/filesystem.hpp>

#include <fmt/format.h>

#include <jank/util/mapped_file.hpp>
#include <jank/util/process_location.hpp>
#include <jank/runtime/core.hpp>
#include <jank/runtime/core/munge.hpp>
#include <jank/runtime/core/truthy.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/obj/atom.hpp>
#include <jank/runtime/obj/jit_function.hpp>
#include <jank/runtime/obj/native_function_wrapper.hpp>
#include <jank/runtime/obj/persistent_sorted_set.hpp>
#include <jank/runtime/module/loader.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/profile/time.hpp>
#include <jank/native_persistent_string/fmt.hpp>

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
    static native_persistent_string const dot{ "\\." };
    return runtime::munge_extra(module, dot, "/");
  }

  native_persistent_string module_to_load_function(native_persistent_string_view const &module)
  {
    static native_persistent_string const dot{ "\\." };
    std::string ret{ runtime::munge_extra(module, dot, "_") };

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
      throw std::runtime_error{ fmt::format("Failed to open jar on module path: {}", path) };
    }

    auto const &zip_entry(zf.getEntry(std::string{ entry.path }));
    fn(zip_entry);
  }

  static void register_entry(native_unordered_map<native_persistent_string, loader::entry> &entries,
                             boost::filesystem::path const &module_path,
                             file_entry const &entry)
  {
    boost::filesystem::path const p{ native_transient_string{ entry.path } };
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
      //   fmt::println("register_entry {} {} {} {}",
      //               entry.archive_path.unwrap_or("None"),
      //               entry.path,
      //               module_path.string(),
      //               path_to_module(module_path));
    }
  }

  static void
  register_relative_entry(native_unordered_map<native_persistent_string, loader::entry> &entries,
                          boost::filesystem::path const &resource_path,
                          file_entry const &entry)
  {
    boost::filesystem::path const p{ native_transient_string{ entry.path } };
    /* We need the file path relative to the module path, since the class
     * path portion is not included in part of the module name. For example,
     * the file may live in `src/jank/clojure/core.jank` but the module
     * should be `clojure.core`, not `src.jank.clojure.core`. */
    auto const &module_path(p.lexically_relative(resource_path));
    register_entry(entries, module_path, entry);
  }

  static void
  register_directory(native_unordered_map<native_persistent_string, loader::entry> &entries,
                     boost::filesystem::path const &path)
  {
    for(auto const &f : boost::filesystem::recursive_directory_iterator{ path })
    {
      if(boost::filesystem::is_regular_file(f))
      {
        register_relative_entry(entries, path, file_entry{ none, f.path().string() });
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
      std::cerr << fmt::format("Failed to open jar on module path: {}\n", path);
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
      auto const &module_path(p.string());
      register_entry(entries, module_path, { none, module_path });
    }
  }

  loader::loader(context &rt_ctx, native_persistent_string_view const &ps)
    : rt_ctx{ rt_ctx }
  {
    auto const jank_path(jank::util::process_location().unwrap().parent_path());
    native_transient_string paths{ ps };
    paths += fmt::format(":{}", (jank_path / "classes").string());
    paths += fmt::format(":{}", (jank_path / "../src/jank").string());
    paths += fmt::format(":{}", rt_ctx.binary_cache_dir);
    this->paths = paths;

    // fmt::println("module paths: {}", paths);

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

  native_bool file_entry::exists() const
  {
    auto const is_archive{ archive_path.is_some() };
    if(is_archive && !boost::filesystem::exists(native_transient_string{ archive_path.unwrap() }))
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

      return source_exists || boost::filesystem::exists(native_transient_string{ path });
    }
  }

  std::time_t file_entry::last_modified_at() const
  {
    auto const source_path{ archive_path.unwrap_or(path) };
    return boost::filesystem::last_write_time(native_transient_string{ source_path });
  }

  string_result<loader::find_result>
  loader::find(native_persistent_string_view const &module, origin const ori)
  {
    static std::regex const underscore{ "_" };
    native_transient_string patched_module{ module };
    patched_module = std::regex_replace(patched_module, underscore, "-");
    auto const &entry(entries.find(patched_module));
    if(entry == entries.end())
    {
      return err(fmt::format("unable to find module: {}", module));
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
       * Unlike class files, object files are tied to the OS, architecture, c++ stdlib etc,
       * making it hard to share them. */
      if(entry->second.o.is_some() && entry->second.o.unwrap().archive_path.is_none()
         && entry->second.o.unwrap().exists()
         && (entry->second.jank.is_some() || entry->second.cljc.is_some()))
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
        else
        {
          return err(
            fmt::format("Found a binary ({}), without a source", entry->second.o.unwrap().path));
        }

        if(boost::filesystem::last_write_time(o_file_path) >= source_modified_time)
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

    return err(fmt::format("no sources for registered module: {}", module));
  }

  native_bool loader::is_loaded(native_persistent_string_view const &module)
  {
    auto const atom{
      runtime::try_object<runtime::obj::atom>(__rt_ctx->loaded_libs_var->deref())->deref()
    };

    auto const loaded_libs{ runtime::try_object<runtime::obj::persistent_sorted_set>(atom) };
    return truthy(loaded_libs->contains(make_box<obj::symbol>(module)));
  }

  void loader::set_is_loaded(native_persistent_string_view const &module)
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

  string_result<void> loader::load(native_persistent_string_view const &module, origin const ori)
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

    string_result<void> res(err(fmt::format("Couldn't load module: {}", module)));

    auto const module_type_to_load{ found_module.expect_ok().to_load.unwrap() };
    auto const &module_sources{ found_module.expect_ok().sources };

    switch(module_type_to_load)
    {
      case module_type::jank:
        res = load_jank(module_sources.jank.unwrap());
        break;
      case module_type::o:
        res = load_o(module, module_sources.o.unwrap());
        break;
      case module_type::cpp:
        res = load_cpp(module_sources.cpp.unwrap());
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
    profile::timer const timer{ fmt::format("load object {}", module) };

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

    if(entry.archive_path.is_some())
    {
      /* TODO: Load object code from string. */
      //visit_jar_entry(entry, [&](auto const &str) { rt_ctx.jit_prc.load_object(module, str); });
    }
    else
    {
      rt_ctx.jit_prc.load_object(entry.path);
    }

    auto const load{ rt_ctx.jit_prc.find_symbol<object *(*)()>(load_function_name).expect_ok() };
    load();

    return ok();
  }

  string_result<void> loader::load_cpp(file_entry const &entry) const
  {
    if(entry.archive_path.is_some())
    {
      visit_jar_entry(entry, [&](auto const &zip_entry) {
        rt_ctx.jit_prc.eval_string(zip_entry.readAsText());
      });
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

  string_result<void> loader::load_jank(file_entry const &entry) const
  {
    if(entry.archive_path.is_some())
    {
      visit_jar_entry(entry,
                      [&](auto const &zip_entry) { rt_ctx.eval_string(zip_entry.readAsText()); });
    }
    else
    {
      rt_ctx.eval_file(entry.path);
    }

    return ok();
  }

  string_result<void> loader::load_cljc(file_entry const &entry) const
  {
    return loader::load_jank(entry);
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
