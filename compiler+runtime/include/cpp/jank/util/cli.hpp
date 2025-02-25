#pragma once

#include <jank/result.hpp>

namespace jank::util::cli
{
  enum class command : uint8_t
  {
    run,
    compile,
    repl,
    cpp_repl,
    run_main
  };

  struct options
  {
    /* Runtime. */
    native_transient_string module_path;
    native_bool profiler_enabled{};
    native_transient_string profiler_file{ "jank.profile" };
    native_bool gc_incremental{};

    /* Native dependencies. */
    native_vector<native_persistent_string> include_dirs;
    native_vector<native_persistent_string> library_dirs;
    native_vector<native_persistent_string> define_macros;
    native_vector<native_persistent_string> libs;

    /* Compilation. */
    native_integer optimization_level{};

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
