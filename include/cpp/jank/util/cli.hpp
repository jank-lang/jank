#pragma once

#include <jank/result.hpp>

namespace jank::util::cli
{
  enum class command
  {
    run,
    compile,
    repl
  };

  struct options
  {
    /* Runtime. */
    native_string_transient class_path;
    native_bool profiler_enabled{};
    native_string_transient profiler_file{ "jank.profile" };
    native_bool gc_incremental{};

    /* Compilation. */
    native_string_transient compilation_path{ "classes" };
#ifdef JANK_RELEASE
    native_integer optimization_level{ 3 };
#else
    native_integer optimization_level{ 0 };
#endif

    /* Run command. */
    native_string_transient target_file;

    /* Compile command. */
    native_string_transient target_ns;
    native_string_transient target_runtime{ "dynamic" };

    /* REPL command. */
    native_bool repl_server{};

    command command{ command::repl };
  };

  result<options, int> parse(int const argc, char const **argv);
}
