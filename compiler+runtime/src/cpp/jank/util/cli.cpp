#include <CLI/CLI.hpp>

#include <fmt/format.h>

#include <jank/util/cli.hpp>
#include <jank/runtime/module/loader.hpp>

namespace jank::util::cli
{
  result<options, int> parse(int const argc, char const **argv)
  {
    CLI::App cli{ "jank compiler" };
    options opts;

    cli.set_help_flag("-h,--help", "Print this help message and exit.");

    /* Runtime. */
    cli.add_option(
      "--module-path",
      opts.module_path,
      fmt::format(
        "A {} separated list of directories, JAR files, and ZIP files to search for modules.",
        runtime::module::loader::module_separator));
    cli.add_flag("--profile", opts.profiler_enabled, "Enable compiler and runtime profiling.");
    cli.add_option("--profile-output",
                   opts.profiler_file,
                   "The file to write profile entries (will be overwritten).");
    cli.add_flag("--gc-incremental", opts.gc_incremental, "Enable incremental GC collection.");
    cli.add_option("-O,--optimization", opts.optimization_level, "The optimization level to use.")
      ->check(CLI::Range(0, 3));

    /* Native dependencies. */
    cli.add_option("-I,--include-dir",
                   opts.include_dirs,
                   "Absolute or relative path to the directory for includes resolution. Can be "
                   "specified multiple times.");
    cli.add_option("-L,--library-dir",
                   opts.library_dirs,
                   "Absolute or relative path to the directory to search dynamic libraries in. "
                   "Can be specified multiple times.");
    cli.add_option("-D,--define-macro",
                   opts.define_macros,
                   "Defines macro value, sets to 1 if omitted. Can be specified multiple times.");
    cli.add_option("-l",
                   opts.libs,
                   "Library identifiers, absolute or relative paths eg. -lfoo for libfoo.so or "
                   "foo.dylib. Can be specified multiple times.");

    /* Run subcommand. */
    auto &cli_run(*cli.add_subcommand("run", "Load and run a file."));
    cli_run.fallthrough();
    cli_run.add_option("file", opts.target_file, "The entrypoint file.")
      ->check(CLI::ExistingFile)
      ->required();

    /* Compile subcommand. */
    auto &cli_compile(*cli.add_subcommand("compile", "Compile a file and its dependencies."));
    cli_compile.fallthrough();
    cli_compile
      .add_option("--runtime", opts.target_runtime, "The runtime of the compiled program.")
      ->check(CLI::IsMember({ "dynamic", "static" }));
    cli_compile
      .add_option("ns", opts.target_ns, "The entrypoint namespace (must be on module path).")
      ->required();

    /* REPL subcommand. */
    auto &cli_repl(*cli.add_subcommand("repl", "Start up a terminal REPL and optional server."));
    cli_repl.fallthrough();
    cli_repl.add_option("module", opts.target_module, "The entrypoint module.");
    cli_repl.add_flag("--server", opts.repl_server, "Start an nREPL server.");

    /* C++ REPL subcommand. */
    auto &cli_cpp_repl(*cli.add_subcommand("cpp-repl", "Start up a terminal C++ REPL."));

    /* Run subcommand. */
    auto &cli_run_main(*cli.add_subcommand("run-main", "Load and execute -main."));
    cli_run_main.fallthrough();
    cli_run_main.add_option("module", opts.target_module, "The entrypoint module.")->required();

    cli.require_subcommand(1);
    cli.failure_message(CLI::FailureMessage::help);
    cli.allow_extras();

    try
    {
      cli.parse(argc, argv);
    }
    catch(CLI::ParseError const &e)
    {
      return err(cli.exit(e));
    }

    if(cli.remaining_size() >= 0)
    {
      opts.extra_opts = cli.remaining();
    }

    if(cli.got_subcommand(&cli_run))
    {
      opts.command = command::run;
    }
    else if(cli.got_subcommand(&cli_compile))
    {
      opts.command = command::compile;
    }
    else if(cli.got_subcommand(&cli_repl))
    {
      opts.command = command::repl;
    }
    else if(cli.got_subcommand(&cli_cpp_repl))
    {
      opts.command = command::cpp_repl;
    }
    else if(cli.got_subcommand(&cli_run_main))
    {
      opts.command = command::run_main;
    }

    return ok(opts);
  }
}
