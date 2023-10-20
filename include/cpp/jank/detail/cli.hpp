#pragma once

namespace jank
{
  struct cli_options
  {
    /* Runtime. */
    native_string class_path;
    native_bool profiler_enabled{};
    native_string profiler_file{ "jank.profile" };

    /* Compilation. */
    native_string compilation_path{ "classes" };
    native_integer optimization_level{ 0 };

    /* Run command. */
    native_string target_file;

    /* Compile command. */
    native_string target_ns;
    native_string target_runtime{ "dynamic" };

    /* REPL command. */
    native_bool repl_server{};
  };
}
