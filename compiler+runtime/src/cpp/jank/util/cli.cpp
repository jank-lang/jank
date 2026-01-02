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
      ++it;
      if(needs_value)
      {
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
  compile-module              Compile a module (given its namespace) and its dependencies.
  repl                        Start up a terminal REPL and optional server.
  cpp-repl                    Start up a terminal C++ REPL.
  run-main                    Load and execute -main.
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
          --debug             Enable debug symbol generation for generated code.
          --direct-call       Elides the dereferencing of vars for improved performance.
  -O,     --optimization <0 - 3>
                              The optimization level to use for AOT compilation.
          --codegen <llvm-ir, cpp> [default: cpp]
                              The type of code generation to use.
  -I,     --include-dir <path>
                              Absolute or relative path to the directory for includes
                              resolution. Can be specified multiple times.
  -D,     --define-macro <name>
                              Defines a macro. The value will be 1, if omitted. Can be specified
                              multiple times.
  -L,     --library-dir <path>
                              Absolute or relative path to the directory to search dynamic
                              libraries in. Can be specified multiple times.
  -l <lib>                    Library identifiers, absolute or relative paths eg. -lfoo for
                              libfoo.so or foo.dylib. Can be specified multiple times.)",
                  JANK_VERSION);
    std::exit(1);
  }

  jtl::result<void, int> parse_opts(int const argc, char const **argv)
  {
    auto const flags{ parse_into_vector(argc, argv) };

    static native_unordered_map<jtl::immutable_string, command> valid_commands{
      {            "run",            command::run },
      {       "run-main",       command::run_main },
      {           "repl",           command::repl },
      {       "cpp-repl",       command::cpp_repl },
      { "compile-module", command::compile_module },
      {        "compile",        command::compile },
      {   "check-health",   command::check_health }
    };

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
          /* This implies that everything coming after is meant for the running program. */
          std::copy(it, end, std::back_inserter(opts.extra_opts));
          break;
        }
        else if(check_flag(it, end, value, "-h", "--help", false))
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
        else if(check_flag(it, end, value, "--direct-call", false))
        {
          opts.direct_call = true;
        }
        else if(check_flag(it, end, value, "-O", "--optimization", true))
        {
          if(value == "0")
          {
            opts.optimization_level = 0;
          }
          else if(value == "1")
          {
            opts.optimization_level = 1;
          }
          else if(value == "2")
          {
            opts.optimization_level = 2;
          }
          else if(value == "3")
          {
            opts.optimization_level = 3;
          }
          else
          {
            throw util::format("Invalid optimization level '{}'.", value);
          }
        }
        else if(check_flag(it, end, value, "--codegen", true))
        {
          if(value == "cpp")
          {
            opts.codegen = codegen_type::cpp;
          }
          else if(value == "llvm-ir")
          {
            opts.codegen = codegen_type::llvm_ir;
          }
          else
          {
            throw util::format("Invalid codegen type '{}'.", value);
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

        /**** These are command-specific flags which we will store until we know the command. ****/
        else if(check_flag(it, end, value, "-o", "--output", true))
        {
          pending_flags["--output"] = value;
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
        if(check_pending_flag("--output", value, pending_flags))
        {
          if(command == "compile-module")
          {
            opts.output_module_filename = value;
          }
          else
          {
            opts.output_filename = value;
          }
        }
        if(check_pending_flag("--output-target", value, pending_flags))
        {
          if(value == "llvm-ir")
          {
            opts.output_target = compilation_target::llvm_ir;
          }
          else if(value == "cpp")
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
