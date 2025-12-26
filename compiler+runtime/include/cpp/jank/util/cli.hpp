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
    run_main,
    check_health
  };

  enum class codegen_type : u8
  {
    llvm_ir,
    cpp
  };

  constexpr char const *codegen_type_str(codegen_type const type)
  {
    switch(type)
    {
      case codegen_type::llvm_ir:
        return "llvm-ir";
      case codegen_type::cpp:
        return "cpp";
      default:
        return "unknown";
    }
  }

  enum class compilation_target : u8
  {
    /* The target will be determined based on the extension of the output.
     * If that's not possible, we'll error out. */
    unspecified,
    llvm_ir,
    cpp,
    object
  };

  constexpr char const *compilation_target_str(compilation_target const target)
  {
    switch(target)
    {
      case compilation_target::unspecified:
        return "unspecified";
      case compilation_target::llvm_ir:
        return "llvm-ir";
      case compilation_target::cpp:
        return "cpp";
      case compilation_target::object:
        return "object";
      default:
        return "unknown";
    }
  }

  struct options
  {
    /* Runtime. */
    std::string module_path;
    std::string profiler_file{ "jank.profile" };
    bool profiler_enabled{};
    bool perf_profiling_enabled{};
    bool gc_incremental{};
    codegen_type codegen{ codegen_type::cpp };

    /* Native dependencies. */
    native_vector<jtl::immutable_string> include_dirs;
    native_vector<jtl::immutable_string> library_dirs;
    native_vector<jtl::immutable_string> define_macros;
    native_vector<jtl::immutable_string> libs;

    /* Compilation. */
    bool debug{};
    u8 optimization_level{};
    bool direct_call{};

    /* Run command. */
    std::string target_file;

    /* Compile command. */
    std::string target_module;
    std::string target_runtime{ "dynamic" };
    std::string output_filename{ "a.out" };

    /* Compile-module command. */
    std::string output_module_filename;
    compilation_target output_target;

    /* REPL command. */
    bool repl_server{};

    /* Extras.
     * TODO: Use a native_persistent_vector instead.
     * */
    std::vector<std::string> extra_opts;

    command command{ command::repl };
  };

  /* NOLINTNEXTLINE */
  extern options opts;

  jtl::result<void, int> parse(int const argc, char const **argv);
  std::vector<std::string> parse_empty(int const argc, char const **argv);
}
