#include <libzippp.h>

#include <regex>

#include <boost/filesystem.hpp>

#include <jank/runtime/module/loader.hpp>

namespace jank::runtime::module
{
  /* This turns `foo_bar/spam/meow.cljc` into `foo-bar.spam.meow`. */
  native_string path_to_module(boost::filesystem::path const &path)
  {
    static std::regex const underscore{ "_" };
    static std::regex const slash{ "/" };

    auto const &s(path.string());
    std::string ret{ s, 0, s.size() - path.extension().size() };
    ret = std::regex_replace(ret, underscore, "-");
    ret = std::regex_replace(ret, slash, ".");

    return ret;
  }

  void register_entry(native_unordered_map<native_string, loader::entry> &entries, file_entry const &entry)
  {
    boost::filesystem::path p{ entry.path };
    auto const ext(p.extension().string());
    bool registered{};
    if(ext == ".jank")
    {
      registered = true;
      loader::entry e;
      e.jank = entry;
      auto res(entries.insert({ path_to_module(p), std::move(e) }));
      if(!res.second)
      { res.first->second.jank = entry; }
    }
    else if(ext == ".cljc")
    {
      registered = true;
      loader::entry e;
      e.cljc = entry;
      auto res(entries.insert({ path_to_module(p), std::move(e) }));
      if(!res.second)
      { res.first->second.cljc = entry; }
    }
    else if(ext == ".cpp")
    {
      registered = true;
      loader::entry e;
      e.cpp = entry;
      auto res(entries.insert({ path_to_module(p), std::move(e) }));
      if(!res.second)
      { res.first->second.cpp = entry; }
    }
    else if(ext == ".pcm")
    {
      registered = true;
      loader::entry e;
      e.pcm = entry;
      auto res(entries.insert({ path_to_module(p), std::move(e) }));
      if(!res.second)
      { res.first->second.pcm = entry; }
    }

    if(registered)
    { fmt::println("register_entry {} {} {}", entry.archive_path, entry.path.string(), path_to_module(p)); }
  }

  void register_directory(native_unordered_map<native_string, loader::entry> &entries, boost::filesystem::path const &path)
  {
    for(auto const &f : boost::filesystem::recursive_directory_iterator{ path })
    {
      if(boost::filesystem::is_regular_file(f))
      { register_entry(entries, file_entry{ none, f.path() }); }
    }
  }

  void register_jar(native_unordered_map<native_string, loader::entry> &entries, native_string_view const &path)
  {
    fmt::println("register_jar {}", path);
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
      { register_entry(entries, { path, name }); }
    }
  }

  void register_path(native_unordered_map<native_string, loader::entry> &entries, native_string_view const &path)
  {
    fmt::println("register_path {}", path);

    boost::filesystem::path p{ path };
    if(boost::filesystem::is_directory(p))
    { register_directory(entries, p); }
    else if(p.extension().string() == ".jar")
    { register_jar(entries, path); }
    else
    { register_entry(entries, { none, p }); }
  }

  loader::loader(native_string_view const &paths)
  {
    fmt::println("BINGBONG loader: {}", paths);

    size_t start{};
    size_t i{ paths.find(module_separator, start) };

    /* Looks like it's either an empty path list or there's only entry. */
    if(i == native_string_view::npos)
    { register_path(entries, paths); }
    else
    {
      while(i != native_string_view::npos)
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
    return runtime::obj::map::create_unique
    (
      make_box("__type"), make_box("module::file_entry"),
      make_box("archive_path"), jank::detail::to_runtime_data(archive_path),
      make_box("path"), make_box(path.string())
    );
  }

  object_ptr loader::to_runtime_data() const
  {
    runtime::object_ptr entry_maps(make_box<runtime::obj::map>());
    for(auto const &e : entries)
    {
      entry_maps = runtime::assoc
      (
        entry_maps,
        make_box(e.first),
        runtime::obj::map::create_unique
        (
          make_box("jank"), jank::detail::to_runtime_data(e.second.jank),
          make_box("cljc"), jank::detail::to_runtime_data(e.second.cljc),
          make_box("cpp"), jank::detail::to_runtime_data(e.second.cpp),
          make_box("pcm"), jank::detail::to_runtime_data(e.second.pcm)
        )
      );
    }

    return runtime::obj::map::create_unique
    (
      make_box("__type"), make_box("module::loader"),
      make_box("entries"), entry_maps
    );
  }
}
