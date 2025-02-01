#pragma once

#include <boost/filesystem/path.hpp>

#include <jank/runtime/object.hpp>
#include <jank/result.hpp>

namespace jank::runtime
{
  struct context;
}

namespace jank::jit
{
  struct processor;
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

  native_persistent_string path_to_resource(boost::filesystem::path const &path);
  native_persistent_string module_to_resource(native_persistent_string_view const &module);
  native_persistent_string module_to_load_function(native_persistent_string_view const &module);
  native_persistent_string module_to_native_ns(native_persistent_string_view const &orig_module);
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

    loader(context &rt_ctx, native_persistent_string_view const &ps);

    string_result<find_result> find(native_persistent_string_view const &module, origin const ori);
    string_result<void> load(native_persistent_string_view const &module, origin const ori);

    string_result<void>
    load_o(native_persistent_string const &module, file_entry const &entry) const;
    string_result<void> load_cpp(file_entry const &entry) const;
    string_result<void> load_jank(file_entry const &entry) const;
    string_result<void> load_cljc(file_entry const &entry) const;

    object_ptr to_runtime_data() const;

    context &rt_ctx;
    native_persistent_string paths;
    /* TODO: These will need synchonization. */
    /* This maps resources like `foo_bar/spam/meow` to entries
     * tracking relevant files that correspond to that resource like
     * `foo_bar/spam/meow.cljc`. */
    native_unordered_map<native_persistent_string, entry> entries;
  };
}
