#pragma once

#include <filesystem>

#include <folly/Synchronized.h>

#include <jtl/result.hpp>

#include <jank/runtime/object.hpp>
#include <jank/error.hpp>

namespace jank::runtime
{
  struct context;
}

namespace jank::runtime::module
{
  enum class origin : u8
  {
    /* Regardless of which binaries are present, and how new they are,
     * this will always select the source. */
    source,
    /* Will choose a binary or the source, depending on which is newer.
     * If both exist and are equally new, the binary is preferred. */
    latest,
  };

  enum class module_type : u8
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
    object_ref to_runtime_data() const;
    bool exists() const;
    std::time_t last_modified_at() const;

    /* If the file is within a JAR, this will be the path to the JAR. */
    jtl::option<jtl::immutable_string> archive_path;
    /* If there's an archive path, this path is within the archive. Otherwise, it's the
     * filesystem path. */
    jtl::immutable_string path;
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
    file_view(jtl::immutable_string const &file, int const f, char const * const h, usize const s);
    file_view(jtl::immutable_string const &file, jtl::immutable_string const &buff);
    ~file_view();

    char const *data() const;
    usize size() const;
    jtl::immutable_string const &file_path() const;

    file_view &operator=(file_view const &) = delete;
    file_view &operator=(file_view &&fv) noexcept;

    jtl::immutable_string_view view() const;

  private:
    void reset();

    /* In the case where we map a file, we track this information so we can read it and
     * later unmap it. */
    int fd{};
    char const *head{};
    usize len{};

    /* In the case where we're not mapping, such as when we read the file from a JAR,
     * we'll just have the data instead. Checking data.empty() is how we know which
     * of these cases to follow. */
    jtl::immutable_string buff;

    /* In either case, we keep track of the file name. */
    jtl::immutable_string file;
  };

  jtl::immutable_string path_to_module(std::filesystem::path const &path);
  jtl::immutable_string module_to_path(jtl::immutable_string const &module);
  jtl::immutable_string module_to_load_function(jtl::immutable_string const &module);
  jtl::immutable_string
  nest_module(jtl::immutable_string const &module, jtl::immutable_string const &sub);
  jtl::immutable_string
  nest_native_ns(jtl::immutable_string const &native_ns, jtl::immutable_string const &end);
  bool is_nested_module(jtl::immutable_string const &module);
  jtl::immutable_string module_to_native_ns(jtl::immutable_string const &orig_module);

  /* A core module is one baked into the jank runtime. For example, clojure.core. */
  bool is_core_module(jtl::immutable_string const &module);

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

      jtl::option<file_entry> o;
      jtl::option<file_entry> cpp;
      jtl::option<file_entry> jank;
      jtl::option<file_entry> cljc;
    };

    struct find_result
    {
      /* All the sources for a module */
      entry sources;
      /* On the basis of origin, source that should be loaded. */
      jtl::option<module_type> to_load;
    };

    /* These separators match what the JVM does on each system. */
#ifdef _WIN32
    static constexpr char module_separator{ ';' };
    static constexpr char const *module_separator_name{ "semicolon" };
#else
    static constexpr char module_separator{ ':' };
    static constexpr char const *module_separator_name{ "colon" };
#endif

    loader();

    static jtl::result<file_view, error_ref> read_file(jtl::immutable_string const &path);
    jtl::result<file_view, error_ref> read_module(jtl::immutable_string const &module);

    jtl::result<find_result, error_ref> find(jtl::immutable_string const &module, origin const ori);

    bool is_loaded(jtl::immutable_string const &module);
    void set_is_loaded(jtl::immutable_string const &module);

    jtl::result<void, error_ref> load(jtl::immutable_string const &module, origin const ori);
    jtl::result<void, error_ref>
    load_o(jtl::immutable_string const &module, file_entry const &entry) const;
    jtl::result<void, error_ref>
    load_cpp(jtl::immutable_string const &module, file_entry const &entry) const;
    jtl::result<void, error_ref> load_jank(file_entry const &entry) const;
    jtl::result<void, error_ref> load_cljc(file_entry const &entry) const;

    /* This only adds a single path, so it's assumed there's no separator present. */
    void add_path(jtl::immutable_string const &path);

    object_ref to_runtime_data() const;

    struct mutable_state
    {
      jtl::immutable_string paths;
      /* This maps module strings to entries. Module strings are like fully qualified namespace
     * names. For example, `clojure.core`, `jank.compiler`, etc. */
      native_unordered_map<jtl::immutable_string, entry> entries;
    };

    /*** XXX: Everything here is thread-safe. ***/
    folly::Synchronized<mutable_state> state;
  };
}
