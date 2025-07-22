#pragma once

#include <jtl/result.hpp>

namespace jank::util::cli
{
  enum class command : u8
  {
    run,
    compile,
    compile_module,
    repl,
    cpp_repl,
    run_main
  };

  enum class codegen_type : u8
  {
    llvm_ir,
    cpp
  };

  struct options
  {
    /* Runtime. */
    std::string module_path;
    std::string profiler_file{ "jank.profile" };
    bool profiler_enabled{};
    bool perf_profiling_enabled{};
    bool gc_incremental{};
    codegen_type codegen{ codegen_type::llvm_ir };

    /* Native dependencies. */
    native_vector<jtl::immutable_string> include_dirs;
    native_vector<jtl::immutable_string> library_dirs;
    native_vector<jtl::immutable_string> define_macros;
    native_vector<jtl::immutable_string> libs;

    /* Compilation. */
    bool debug{};
    u8 optimization_level{};

    /* Run command. */
    std::string target_file;

    /* Compile command. */
    std::string target_module;
    std::string target_runtime{ "dynamic" };

    /* REPL command. */
    bool repl_server{};

    /* Extras.
     * TODO: Use a native_persistent_vector instead.
     * */
    std::vector<std::string> extra_opts;

    std::string output_filename;

    command command{ command::repl };
  };

  /* NOLINTNEXTLINE */
  extern options opts;

  jtl::result<void, int> parse(int const argc, char const **argv);
  std::vector<std::string> parse_empty(int const argc, char const **argv);
}
