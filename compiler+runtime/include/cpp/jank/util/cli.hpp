#pragma once

#include <jank/result.hpp>

namespace jank::util::cli
{
  enum class command
  {
    run,
    compile,
    repl,
    run_main
  };

  struct options
  {
    /* Runtime. */
    native_transient_string class_path;
    native_bool profiler_enabled{};
    native_transient_string profiler_file{ "jank.profile" };
    native_bool gc_incremental{};

    /* Compilation. */
    native_transient_string compilation_path{ "classes" };
#ifdef JANK_RELEASE
    /* TODO: Make this 0 when we can.
     * Right now, Clang requires us to have JIT optimizations enabled if jank was
     * compiled with AOT optimizations. This is unforunate, since JIT -O1 is *significantly*
     * slower to compile than -O0. Something like 3x slower, based on my experience. */
    native_integer optimization_level{ 1 };
#else
    native_integer optimization_level{ 0 };
#endif

    /* Run command. */
    native_transient_string target_file;

    /* Compile command. */
    native_transient_string target_ns;
    native_transient_string target_runtime{ "dynamic" };

    /* REPL command. */
    native_bool repl_server{};

    /* Run main command. */
    native_transient_string target_module;

    /* Extras.
     * TODO: Use a native_persistent_vector instead.
     * */
    std::vector<native_transient_string> extra_opts;

    command command{ command::repl };
  };

  result<options, int> parse(int const argc, char const **argv);
}
