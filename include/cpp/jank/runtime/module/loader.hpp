#pragma once

#include <boost/filesystem/path.hpp>

namespace jank::runtime::module
{
  struct file_entry
  {
    object_ptr to_runtime_data() const;

    /* If the file is within a JAR, this will be the path to the JAR. */
    option<native_string> archive_path;
    /* If there's an archive path, this path is within the archive. Otherwise, it's the
     * filesystem path. */
    boost::filesystem::path path;
  };

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
      option<file_entry> jank;
      option<file_entry> cljc;
      option<file_entry> cpp;
      option<file_entry> pcm;
    };

    /* These separators match what the JVM does on each system. */
#ifdef _WIN32
    static constexpr char module_separator{ ';' };
#else
    static constexpr char module_separator{ ':' };
#endif

    loader() = default;
    loader(native_string_view const &paths);

    object_ptr to_runtime_data() const;

    /* This maps module strings to entries. Module strings are like fully qualified Java
     * class names. */
    native_unordered_map<native_string, entry> entries;
  };
}
