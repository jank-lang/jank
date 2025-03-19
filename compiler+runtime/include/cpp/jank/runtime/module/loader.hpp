#pragma once

#include <filesystem>

#include <jank/runtime/object.hpp>
#include <jank/result.hpp>

namespace jank::runtime
{
  struct context;
}

namespace jank::runtime::module
{
  enum class origin : uint8_t
  {
    /* Regardless of which binaries are present, and how new they are,
     * this will always select the source. */
    source,
    /* Will choose a binary or the source, depending on which is newer.
     * If both exist and are equally new, the binary is preferred. */
    latest,
  };

  enum class module_type : uint8_t
  {
    o,
    cpp,
    jank,
    cljc
  };

  constexpr char const *module_type_str(module_type const t)
  {
    switch(t)
    {
      case module_type::o:
        return "o";
      case module_type::cpp:
        return "cpp";
      case module_type::jank:
        return "jank";
      case module_type::cljc:
        return "cljc";
    }
    return "unknown";
  }

  struct file_entry
  {
    object_ptr to_runtime_data() const;
    native_bool exists() const;
    std::time_t last_modified_at() const;

    /* If the file is within a JAR, this will be the path to the JAR. */
    option<native_persistent_string> archive_path;
    /* If there's an archive path, this path is within the archive. Otherwise, it's the
     * filesystem path. */
    native_persistent_string path;
  };

  /* When reading a file, we may find it on the filesystem or within a JAR. In the
   * first case, we map it with `mmap`, but we can't do that for JAR files since
   * we need to decompress them into memory. This `file_view` gives us one view
   * into either type of file, with the same interface. */
  struct file_view
  {
    file_view() = default;
    file_view(file_view const &) = delete;
    file_view(file_view &&) noexcept;
    file_view(int const f, char const * const h, size_t const s);
    file_view(native_persistent_string const &buff);
    ~file_view();

    char const *data() const;
    size_t size() const;

    native_persistent_string_view view() const;

  private:
    /* In the case where we map a file, we track this information so we can read it and
     * later unmap it. */
    int fd{};
    char const *head{};
    size_t len{};

    /* In the case where we're not mapping, such as when we read the file from a JAR,
     * we'll just have the data instead. Checking data.empty() is how we know which
     * of these cases to follow. */
    native_persistent_string buff;
  };

  native_persistent_string path_to_module(std::filesystem::path const &path);
  native_persistent_string module_to_path(native_persistent_string const &module);
  native_persistent_string module_to_load_function(native_persistent_string const &module);
  native_persistent_string
  nest_module(native_persistent_string const &module, native_persistent_string const &sub);
  native_persistent_string
  nest_native_ns(native_persistent_string const &native_ns, native_persistent_string const &end);
  native_bool is_nested_module(native_persistent_string const &module);

  struct loader
  {
    /* A module entry represents one or more files on the module path which prove that module.
     * Modules can either be directly on the filesystem or within JARs.
     *
     * A module entry may not have any file path, yet it may have multiple. For example, a
     * foo.jank path may also have a corresponding foo.cpp path, which has a corresponding foo.pcm
     * path. That's three paths for one module. The case where it lacks any path is when it
     * comes from the REPL.
     *
     * As with the JVM, the first match for a module in the module path is the one taken. Any
     * subsequent matches are ignored. */
    struct entry
    {
      entry() = default;

      option<file_entry> o;
      option<file_entry> cpp;
      option<file_entry> jank;
      option<file_entry> cljc;
    };

    struct find_result
    {
      /* All the sources for a module */
      entry sources;
      /* On the basis of origin, source that should be loaded. */
      option<module_type> to_load;
    };

    /* These separators match what the JVM does on each system. */
#ifdef _WIN32
    static constexpr char module_separator{ ';' };
#else
    static constexpr char module_separator{ ':' };
#endif

    loader(context &rt_ctx, native_persistent_string const &ps);

    static string_result<file_view> read_file(native_persistent_string const &path);

    string_result<find_result> find(native_persistent_string const &module, origin const ori);

    native_bool is_loaded(native_persistent_string const &module);
    void set_is_loaded(native_persistent_string const &module);

    string_result<void> load(native_persistent_string const &module, origin const ori);
    string_result<void>
    load_o(native_persistent_string const &module, file_entry const &entry) const;
    string_result<void>
    load_cpp(native_persistent_string const &module, file_entry const &entry) const;
    string_result<void> load_jank(file_entry const &entry) const;
    string_result<void> load_cljc(file_entry const &entry) const;

    object_ptr to_runtime_data() const;

    context &rt_ctx;
    native_persistent_string paths;
    /* TODO: These will need synchonization. */
    /* This maps module strings to entries. Module strings are like fully qualified Java
     * class names. For example, `clojure.core`, `jank.compiler`, etc. */
    native_unordered_map<native_persistent_string, entry> entries;
  };
}
