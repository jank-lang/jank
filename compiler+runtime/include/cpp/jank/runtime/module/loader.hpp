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
  struct file_entry
  {
    object_ptr to_runtime_data() const;

    /* If the file is within a JAR, this will be the path to the JAR. */
    option<native_persistent_string> archive_path;
    /* If there's an archive path, this path is within the archive. Otherwise, it's the
     * filesystem path. */
    native_persistent_string path;
  };

  native_persistent_string path_to_module(boost::filesystem::path const &path);
  native_persistent_string module_to_path(native_persistent_string_view const &module);
  native_persistent_string module_to_load_function(native_persistent_string_view const &module);
  native_persistent_string module_to_native_ns(native_persistent_string_view const &orig_module);
  native_persistent_string
  nest_module(native_persistent_string const &module, native_persistent_string const &sub);
  native_persistent_string
  nest_native_ns(native_persistent_string const &native_ns, native_persistent_string const &end);
  native_bool is_nested_module(native_persistent_string const &module);

  struct loader
  {
    /* A module entry represents one or more files on the classpath which prove that module.
    * Modules can either be directly on the filesystem or within JARs.
    *
    * A module entry may not have any file path, yet it may have multiple. For example, a
    * foo.jank path may also have a corresponding foo.cpp path, which has a corresponding foo.pcm
    * path. That's three paths for one module. The case where it lacks any path is when it
    * comes from the REPL.
    *
    * As with the JVM, the first match for a module in the classpath is the one taken. Any
    * subsequent matches are ignored. */
    struct entry
    {
      option<file_entry> pcm;
      option<file_entry> cpp;
      option<file_entry> jank;
      option<file_entry> cljc;
    };

    /* These separators match what the JVM does on each system. */
#ifdef _WIN32
    static constexpr char module_separator{ ';' };
#else
    static constexpr char module_separator{ ':' };
#endif

    loader(context &rt_ctx, native_persistent_string_view const &ps);

    native_bool is_loaded(native_persistent_string_view const &) const;
    void set_loaded(native_persistent_string_view const &);
    result<void, native_persistent_string> load_ns(native_persistent_string_view const &module);
    result<void, native_persistent_string> load(native_persistent_string_view const &module);
    result<void, native_persistent_string> load_pcm(file_entry const &entry) const;
    result<void, native_persistent_string> load_cpp(file_entry const &entry) const;
    result<void, native_persistent_string> load_jank(file_entry const &entry) const;
    result<void, native_persistent_string> load_cljc(file_entry const &entry) const;

    object_ptr to_runtime_data() const;

    context &rt_ctx;
    native_persistent_string paths;
    /* TODO: These will need synchonization. */
    /* This maps module strings to entries. Module strings are like fully qualified Java
     * class names. */
    native_unordered_map<native_persistent_string, entry> entries;
    native_set<native_persistent_string> loaded;
  };
}
