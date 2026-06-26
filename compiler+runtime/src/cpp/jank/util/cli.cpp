#include <jank/util/cli.hpp>
#include <jank/util/fmt/print.hpp>
#include <jank/runtime/module/loader.hpp>
#include <jank/error/report.hpp>

namespace jank::util::cli
{
  /* NOLINTNEXTLINE */
  options opts;

  using flag_it = native_vector<jtl::immutable_string>::const_iterator;

  static bool check_flag(flag_it &it,
                         flag_it const end,
                         jtl::immutable_string &out,
                         jtl::immutable_string const &short_flag,
                         jtl::immutable_string const &long_flag,
                         bool const needs_value)
  {
    if(*it == long_flag)
    {
      if(needs_value)
      {
        ++it;
        if(it == end)
        {
          throw util::format("The '{}' flag requires an argument, but one was not provided.",
                             long_flag);
        }
        out = *it;
      }
      return true;
    }
    else if(!short_flag.empty() && (*it).starts_with(short_flag))
    {
      if(needs_value)
      {
        out = (*it).substr(short_flag.size());
        if(out.empty())
        {
          ++it;
          if(it == end)
          {
            throw util::format("The '{}' flag requires an argument, but one was not provided.",
                               short_flag);
          }
          out = *it;
        }
      }
      return true;
    }
    return false;
  }

  static bool check_flag(flag_it &it,
                         flag_it const end,
                         jtl::immutable_string &out,
                         jtl::immutable_string const &long_flag,
                         bool const needs_value)
  {
    return check_flag(it, end, out, "", long_flag, needs_value);
  }

  static jtl::immutable_string
  get_positional_arg(jtl::immutable_string const &command,
                     jtl::immutable_string const &name,
                     native_deque<jtl::immutable_string> &pending_positional_args)
  {
    if(pending_positional_args.empty())
    {
      throw util::format("Please provide the {} command a {}.", command, name);
    }

    jtl::immutable_string ret{ pending_positional_args.front() };
    pending_positional_args.pop_front();
    return ret;
  }

  static bool check_pending_flag(
    jtl::immutable_string const &long_flag,
    jtl::immutable_string &out,
    native_unordered_map<jtl::immutable_string, jtl::immutable_string> &pending_flags)
  {
    auto const found{ pending_flags.find(long_flag) };
    if(found == pending_flags.end())
    {
      return false;
    }
    out = found->second;
    pending_flags.erase(found);
    return true;
  }

  static void show_help()
  {
    /* TODO: Support command help. */
    /* TODO: Provide examples. */
    /* TODO: Improve layout and required arg indicators. */
    /* TODO: Colorize output. */
    util::println(R"(
jank compiler {}

The jank compiler is used to evaluate and compile jank, Clojure, and C++ sources.

COMMANDS
  run                         Load and run a file.
  repl                        Start up a terminal REPL client and server.
  cpp-repl                    Start up a terminal C++ REPL client.
  run-main                    Load and execute -main.
  compile-module              Ahead of time compile a module (given its namespace) and its
                              dependencies.
  compile                     Ahead of time compile project with entrypoint module containing
                              -main.
  check-health                Provide a status report on the jank installation.

OPTIONS
  -h,     --help              Print this help message and exit.
          --module-path <path>
                              A colon separated list of directories, JAR files, and ZIP files
                              to search for modules.
          --profile           Enable compiler and runtime profiling.
          --profile-output <path> [default: jank.profile]
                              The file to write profile entries (will be overwritten).
          --perf              Enable Linux perf event sampling.
          --gc-incremental    Enable incremental GC collection.
          --no-debug          Disable debug source map generation for generated code.
  -O,     --optimization <0 - 3>
                              The optimization level to use for AOT compilation.
  -Odirect-call               Elides the dereferencing of vars for improved performance. (not yet implemented)
          --eagerness <lazy, eager> [default: lazy]
                              How eagerly to JIT compile functions.
          --runtime <static, dynamic> [default: static]
                              The AOT runtime to target. The static runtime bakes in
                              all functionality and does not link to Clang/LLVM for easier
                              distribution.
          --target-dir <path> [default: target]
                              The directory to use for storing final artifacts.
          --build-dir <path> [default: <target dir>/_cache]
                              The prefix to use for intermediate build files.
          --name <name> [default: a.out]
                              The name of the output file, in the target directory.
          --output-target <cpp, object> [default: object]
                              The target of each compiled module artifact.
  -I,     --include-dir <path>
                              Absolute or relative path to the directory for includes
                              resolution. Can be specified multiple times.
  -D,     --define-macro <name>
                              Defines a macro. The value will be 1, if omitted. Can be specified
                              multiple times. (example: -DFOO -DBAR=meow)
  -L,     --library-dir <path>
                              Absolute or relative path to the directory to search dynamic
                              libraries in. Can be specified multiple times.
  -l <lib>                    Library identifiers, absolute or relative paths eg. -lfoo for
                              libfoo.so or foo.dylib. Can be specified multiple times.)",
                  JANK_VERSION);
    std::exit(1);
  }

  struct options_scratchpad
  {
    /* TODO: Enum for these. */
    jtl::option<u8> runtime_optimization_level;
    jtl::option<u8> codegen_optimization_level;

    jtl::option<jtl::immutable_string> build_dir;

    /* Optimization passes. */
    /*** O1 ***/
    jtl::option<bool> hoist_literals;
    jtl::option<bool> remove_nops;

    /*** O2 ***/

    /*** O3 ***/
    jtl::option<bool> hoist_var_derefs;

    /* Other optimization flags. */
    jtl::option<bool> direct_call;
  };

  static native_unordered_map<jtl::immutable_string, jtl::option<bool>(options_scratchpad::*)> const
    optimization_flags{
      {   "hoist-literals",   &options_scratchpad::hoist_literals },
      {      "remove-nops",      &options_scratchpad::remove_nops },
      { "hoist-var-derefs", &options_scratchpad::hoist_var_derefs },
      {      "direct-call",      &options_scratchpad::direct_call },
  };

  /* TODO: Construct global options here. */
  static void commit_scratchpad(options_scratchpad const &scratch)
  {
    opts.runtime_optimization_level = scratch.runtime_optimization_level.unwrap_or(0);
    opts.codegen_optimization_level = scratch.codegen_optimization_level.unwrap_or(0);
    opts.direct_call = scratch.direct_call.unwrap_or(false);

    opts.build_dir = scratch.build_dir.unwrap_or(util::format("{}/_cache", opts.target_dir));

    /* NOLINTNEXTLINE(bugprone-switch-missing-default-case) */
    switch(opts.codegen_optimization_level)
    {
      case 3:
        opts.hoist_var_derefs = scratch.hoist_var_derefs.unwrap_or(true);
        [[fallthrough]];
      case 2:
        [[fallthrough]];
      case 1:
        opts.hoist_literals = scratch.hoist_literals.unwrap_or(true);
        opts.remove_nops = scratch.remove_nops.unwrap_or(true);
        break;
    }
  }

  jtl::result<void, int> parse_opts(int const argc, char const **argv)
  {
    auto const flags{ parse_into_vector(argc, argv) };

    static native_unordered_map<jtl::immutable_string, command> valid_commands{
      {            "run",            command::run },
      {       "run-main",       command::run_main },
      {           "repl",           command::repl },
      {       "cpp-repl",       command::cpp_repl },
      {        "compile",        command::compile },
      { "compile-module", command::compile_module },
      {   "check-health",   command::check_health }
    };

    options_scratchpad scratch;

    /* The flow of this is broken into the following steps.
     *
     * 1. Parse known flags into changes to the `opts` global
     * 2. Determine the first positional arg to be the command
     * 3. Store other positional args to be handled later (pending)
     * 4. Store unhandled flags to be handled later (pending)
     * 5. Once we've gone through everything, use the command we have
     *    to pick apart the pending flags and positional args
     * 6. Complain about anything left over */
    try
    {
      native_unordered_map<jtl::immutable_string, jtl::immutable_string> pending_flags;
      native_deque<jtl::immutable_string> pending_positional_args;
      auto const end{ flags.end() };
      jtl::immutable_string command;
      jtl::immutable_string value;

      for(auto it{ flags.begin() }; it != end; ++it)
      {
        /**** These are all of the global flags which can apply to any command. ****/
        if(check_flag(it, end, value, "--", false))
        {
          ++it;
          /* This implies that everything coming after is meant for the running program. */
          std::copy(it, end, std::back_inserter(opts.extra_opts));
          break;
        }

        bool handled{};
        /* We automatically support flags like -Ofoo and -Ono-foo here. */
        for(auto const &pass : optimization_flags)
        {
          if(check_flag(it, end, value, util::format("-O{}", pass.first), false))
          {
            scratch.*(pass.second) = true;
            handled = true;
            break;
          }
          else if(check_flag(it, end, value, util::format("-Ono-{}", pass.first), false))
          {
            scratch.*(pass.second) = false;
            handled = true;
            break;
          }
        }
        if(handled)
        {
          continue;
        }

        if(check_flag(it, end, value, "-h", "--help", false))
        {
          show_help();
        }
        else if(check_flag(it, end, value, "--module-path", true))
        {
          opts.module_path = value;
        }
        else if(check_flag(it, end, value, "--profile", false))
        {
          opts.profiler_enabled = true;
        }
        else if(check_flag(it, end, value, "--profile-output", true))
        {
          opts.profiler_file = value;
        }
        else if(check_flag(it, end, value, "--perf", false))
        {
          opts.perf_profiling_enabled = true;
        }
        else if(check_flag(it, end, value, "--no-debug", false))
        {
          opts.debug = false;
        }
        else if(check_flag(it, end, value, "-O", "--optimization", true))
        {
          if(value == "0")
          {
            scratch.codegen_optimization_level = static_cast<u8>(0);
          }
          else if(value == "1")
          {
            scratch.codegen_optimization_level = static_cast<u8>(1);
          }
          else if(value == "2")
          {
            scratch.codegen_optimization_level = static_cast<u8>(2);
          }
          else if(value == "3")
          {
            scratch.codegen_optimization_level = static_cast<u8>(3);
          }
          else
          {
            throw util::format("Invalid optimization level '{}'.", value);
          }
        }
        else if(check_flag(it, end, value, "--eagerness", true))
        {
          if(value == "lazy")
          {
            opts.eagerness = compilation_eagerness::lazy;
          }
          else if(value == "eager")
          {
            opts.eagerness = compilation_eagerness::eager;
          }
          else
          {
            throw util::format("Invalid eagerness type '{}'.", value);
          }
        }
        else if(check_flag(it, end, value, "--runtime", true))
        {
          if(value == "static")
          {
            opts.target_runtime = compilation_runtime::static_;
          }
          else if(value == "dynamic")
          {
            opts.target_runtime = compilation_runtime::dynamic;
          }
          else
          {
            throw util::format("Invalid runtime type '{}'.", value);
          }
        }
        else if(check_flag(it, end, value, "-I", "--include-dir", true))
        {
          opts.include_dirs.emplace_back(value);
        }
        else if(check_flag(it, end, value, "-D", "--define-macro", true))
        {
          opts.define_macros.emplace_back(value);
        }
        else if(check_flag(it, end, value, "-L", "--library-dir", true))
        {
          opts.library_dirs.emplace_back(value);
        }
        else if(check_flag(it, end, value, "-l", "--link", true))
        {
          opts.libs.emplace_back(value);
        }
        else if(check_flag(it, end, value, "--target-dir", true))
        {
          opts.target_dir = value;
        }
        else if(check_flag(it, end, value, "--build-dir", true))
        {
          scratch.build_dir = value;
        }

        /**** These are command-specific flags which we will store until we know the command. ****/
        else if(check_flag(it, end, value, "--name", true))
        {
          pending_flags["--name"] = value;
        }
        else if(check_flag(it, end, value, "--output-target", true))
        {
          pending_flags["--output-target"] = value;
        }
        else if(check_flag(it, end, value, "--runtime", true))
        {
          pending_flags["--runtime"] = value;
        }
        else if(command.empty())
        {
          command = *it;
          auto const found_command{ valid_commands.find(command) };
          if(found_command == valid_commands.end())
          {
            throw util::format("Invalid command '{}'.", command);
          }
          opts.command = found_command->second;
        }
        else
        {
          pending_positional_args.emplace_back(*it);
        }
      }

      if(command.empty())
      {
        show_help();
      }

      /* Now process all pending flags, depending on our command. */
      if(command == "run")
      {
        opts.target_file = get_positional_arg(command, "file", pending_positional_args);
      }
      else if(command == "run-main")
      {
        opts.target_module = get_positional_arg(command, "module", pending_positional_args);
      }
      else if(command == "repl")
      {
        if(!pending_positional_args.empty())
        {
          opts.target_module = get_positional_arg(command, "module", pending_positional_args);
        }
      }
      else if(command == "compile-module" || command == "compile")
      {
        opts.target_module = get_positional_arg(command, "module", pending_positional_args);
        if(check_pending_flag("--output-target", value, pending_flags))
        {
          if(value == "cpp")
          {
            opts.output_target = compilation_target::cpp;
          }
          else if(value == "object")
          {
            opts.output_target = compilation_target::object;
          }
          else
          {
            throw util::format("Invalid output type '{}'.", value);
          }
        }
        if(check_pending_flag("--runtime", value, pending_flags))
        {
          opts.target_file = value;
        }

        if(command == "compile" && opts.output_target == compilation_target::unspecified)
        {
          opts.output_target = compilation_target::object;
        }
      }

      /* We allow --name to be passed for any command, since lein-jank passes it.*/
      if(check_pending_flag("--name", value, pending_flags))
      {
        if(value.contains('/'))
        {
          throw util::format("The argument to --name must be a file name, not a directory.");
        }

        if(command == "compile-module")
        {
          opts.output_module_filename = value;
        }
        else
        {
          opts.output_filename = value;
        }
      }

      if(command == "run" || command == "run-main" || command == "repl")
      {
        scratch.runtime_optimization_level = scratch.codegen_optimization_level;
      }

      /* If we have any more pending flags at this point, they don't belong. */
      if(!pending_flags.empty())
      {
        jtl::string_builder sb;
        util::format_to(sb, "These flags were not used:");
        for(auto const &flag : pending_flags)
        {
          util::format_to(sb, " {}", flag.first);
          if(!flag.second.empty())
          {
            util::format_to(sb, " {}", flag.second);
          }
        }
        error::warn(sb.release());
      }
      else if(!pending_positional_args.empty())
      {
        jtl::string_builder sb;
        util::format_to(sb, "Extra positional args:");
        for(auto const &arg : pending_positional_args)
        {
          util::format_to(sb, " {}", arg);
        }
        throw sb.release();
      }

      commit_scratchpad(scratch);
    }
    catch(jtl::immutable_string const &msg)
    {
      util::println(stderr, "error: {}", msg);
      return err(1);
    }

    return ok();
  }

  native_vector<jtl::immutable_string> parse_into_vector(int const argc, char const **argv)
  {
    native_vector<jtl::immutable_string> ret;
    ret.reserve(argc - 1);
    for(int i{ 1 }; i < argc; ++i)
    {
      ret.emplace_back(argv[i]);
    }
    return ret;
  }
}
