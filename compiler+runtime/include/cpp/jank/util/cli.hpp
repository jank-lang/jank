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
    check_health,
    print_binary_version,
  };

  enum class compilation_target : u8
  {
    /* The target will be determined based on the extension of the output.
     * If that's not possible, we'll error out. */
    unspecified,
    cpp,
    object
  };

  constexpr char const *compilation_target_str(compilation_target const target)
  {
    switch(target)
    {
      case compilation_target::unspecified:
        return "unspecified";
      case compilation_target::cpp:
        return "cpp";
      case compilation_target::object:
        return "object";
      default:
        return "unknown";
    }
  }

  constexpr char const *compilation_target_extension(compilation_target const target)
  {
    switch(target)
    {
      case compilation_target::unspecified:
        return "unspecified";
      case compilation_target::cpp:
        return "cpp";
      case compilation_target::object:
        return "o";
      default:
        return "unknown";
    }
  }

  enum class compilation_eagerness : u8
  {
    lazy,
    eager,
    /* TODO: We can support a batch mode which lazily creates proxy fns during eval
     * and then batches them all together into one compilation to replace the proxies.
     * This would then be the default for run and run-main, whereas lazy would be
     * the default for everything else. */
  };

  constexpr char const *compilation_eagerness_str(compilation_eagerness const eagerness)
  {
    switch(eagerness)
    {
      case compilation_eagerness::lazy:
        return "lazy";
      case compilation_eagerness::eager:
        return "eager";
      default:
        return "unknown";
    }
  }

  enum class compilation_runtime : u8
  {
    static_,
    dynamic
  };

  constexpr char const *compilation_runtime_str(compilation_runtime const rt)
  {
    switch(rt)
    {
      case compilation_runtime::static_:
        return "static";
      case compilation_runtime::dynamic:
        return "dynamic";
      default:
        return "unknown";
    }
  }

  struct options
  {
    /* Runtime. */
    jtl::immutable_string module_path;
    jtl::immutable_string profiler_file{ "jank.profile" };
    bool profiler_enabled{};
    bool perf_profiling_enabled{};
    bool gc_incremental{};

    /* Native dependencies. */
    native_vector<jtl::immutable_string> include_dirs;
    native_vector<jtl::immutable_string> library_dirs;
    native_vector<jtl::immutable_string> define_macros;
    native_vector<jtl::immutable_string> libs;

    /* Compilation. */
    bool debug{ true };

    /* The level of optimization used by the JIT runtime. This will be passed to Clang as a -O
     * flag. It generally only makes when using the repl/run/run-main commands, since during
     * the compile/compile-module commands, we don't care as much how fast the runtime is. */
    u8 runtime_optimization_level{};
    /* The level of optimization used during code generation, for both AOT and JIT compiled code.
     * This is what determines which IR passes we run on the jank functions we compile. */
    u8 codegen_optimization_level{};

    /* Optimization passes. */
    /*** O1 ***/
    bool hoist_literals{};
    bool remove_nops{};

    /*** O2 ***/

    /*** O3 ***/
    bool hoist_var_derefs{};

    /* Other optimization flags. */
    bool direct_call{};
    compilation_eagerness eagerness{ compilation_eagerness::lazy };
    compilation_runtime target_runtime{ compilation_runtime::static_ };

    /* Run command. */
    jtl::immutable_string target_file;

    /* Compile command. */
    jtl::immutable_string target_module;
    jtl::immutable_string output_filename{ "a.out" };
    jtl::immutable_string target_dir{ "target" };
    jtl::immutable_string build_dir;
    jtl::immutable_string forced_binary_version;

    /* Compile-module command. */
    jtl::immutable_string output_module_filename;
    compilation_target output_target{ compilation_target::unspecified };

    /* Extra flags, which will be passed to main. */
    native_vector<jtl::immutable_string> extra_opts;

    command command{ command::repl };
  };

  /* NOLINTNEXTLINE */
  extern options opts;

  /* Affects the global opts. */
  jtl::result<void, int> parse_opts(int const argc, char const **argv);

  /* Takes the CLI args and puts 'em in a vector. */
  native_vector<jtl::immutable_string> parse_into_vector(int const argc, char const **argv);
}
