#include <iostream>
#include <filesystem>

#include <boost/filesystem.hpp>

#include <cling/Interpreter/Interpreter.h>
#include <cling/Interpreter/Value.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Lex/Preprocessor.h>

#include <CLI/CLI.hpp>

#include <jank/runtime/detail/object_util.hpp>

#include <jank/util/mapped_file.hpp>
#include <jank/read/lex.hpp>
#include <jank/read/parse.hpp>
#include <jank/runtime/context.hpp>
#include <jank/analyze/processor.hpp>
#include <jank/codegen/processor.hpp>
#include <jank/evaluate.hpp>
#include <jank/jit/processor.hpp>

namespace jank
{
  void run(cli_options const &opts, runtime::context &rt_ctx)
  {
    {
      profile::timer timer{ "require clojure.core" };
      rt_ctx.load_module("clojure.core").expect_ok();
    }

    //{
    //  auto const mfile(util::map_file(file));
    //  auto const asts(rt_ctx.analyze_string({ mfile.expect_ok().head, mfile.expect_ok().size }, jit_prc));

    //  for(auto const &ast : asts)
    //  {
    //    if(auto *f = boost::get<analyze::expr::function<analyze::expression>>(&ast->data))
    //    {
    //      codegen::processor cg_prc{ rt_ctx, *f };
    //      std::cout << cg_prc.declaration_str() << std::endl;
    //    }
    //    else
    //    {
    //      auto const wrapped(evaluate::wrap_expression(ast));
    //      codegen::processor cg_prc{ rt_ctx, wrapped };
    //      std::cout << cg_prc.declaration_str() << std::endl;
    //    }
    //  }

    //  return 0;
    //}

    {
      profile::timer timer{ "eval user code" };
      std::cout << runtime::detail::to_string(rt_ctx.eval_file(opts.target_file)) << std::endl;
    }
  }

  void compile(cli_options const &opts, runtime::context &rt_ctx)
  {
    rt_ctx.compile_module(opts.target_ns);
  }

  void repl(cli_options const &opts, runtime::context &rt_ctx)
  {
    /* TODO: REPL server. */
    if(opts.repl_server)
    { throw std::runtime_error{ "Not yet implemented: REPL server" }; }

    {
      profile::timer timer{ "require clojure.core" };
      rt_ctx.load_module("clojure.core").expect_ok();
    }

    /* TODO: Readline with history. */
    /* TODO: Completion. */
    /* TODO: Syntax highlighting. */
    std::string line;
    std::cout << "> " << std::flush;
    while(std::getline(std::cin, line))
    {
      try
      {
        auto const res(rt_ctx.eval_string(line));
        fmt::println("{}", runtime::detail::to_string(res));
      }
      /* TODO: Unify error handling. JEEZE! */
      catch(std::exception const &e)
      { fmt::println("Exception: {}", e.what()); }
      catch(jank::runtime::object_ptr const o)
      { fmt::println("Exception: {}", jank::runtime::detail::to_string(o)); }
      catch(jank::native_string const &s)
      { fmt::println("Exception: {}", s); }
      catch(jank::read::error const &e)
      { fmt::println("Read error: {}", e.message); }

      std::cout << "> " << std::flush;
    }
  }
}

// NOLINTNEXTLINE(bugprone-exception-escape): This can only happen if we fail to report an error.
int main(int const argc, char const **argv)
try
{
  using namespace jank;

  /* The GC needs to enabled even before arg parsing, since our native types,
   * like strings, use the GC for allocations. It can still be configured later. */
  GC_set_all_interior_pointers(1);
  GC_enable();
  /* TODO: This crashes now, with LLVM13. Looks like it's cleaning up things it shouldn't. */
  //GC_enable_incremental();

  /* TODO: Apply global options to all subcommands. */
  CLI::App cli{ "jank compiler" };
  cli_options opts;

  /* Runtime. */
  cli.add_option("--class-path", opts.class_path, fmt::format("A {} separated list of directories, JAR files, and ZIP files to search for modules", runtime::module::loader::module_separator));
  cli.add_flag("--profile", opts.profiler_enabled, "Enable compiler and runtime profiling");
  cli.add_option("--profile-output", opts.profiler_file, "The file to write profile entries (will be overwritten)");

  /* Compilation. */
  cli.add_option("--output-dir", opts.compilation_path, "The base directory where compiled modules are written");
  cli.add_option("-O,--optimization", opts.optimization_level, "The optimization level to use")->check(CLI::Range(0, 3));

  /* Run subcommand. */
  auto &cli_run(*cli.add_subcommand("run", "Load and run a file"));
  cli_run.fallthrough();
  cli_run.add_option("file", opts.target_file, "The entrypoint file")->check(CLI::ExistingFile)->required();

  /* Compile subcommand. */
  auto &cli_compile(*cli.add_subcommand("compile", "Compile a file and its dependencies"));
  cli_compile.fallthrough();
  cli_compile.add_option("--runtime", opts.target_runtime, "The runtime of the compiled program")->check(CLI::IsMember({ "dynamic", "static" }));
  cli_compile.add_option("ns", opts.target_ns, "The entrypoint namespace (must be on class path)")->required();

  /* REPL subcommand. */
  auto &cli_repl(*cli.add_subcommand("repl", "Start up a terminal REPL and optional server"));
  cli_repl.fallthrough();
  cli_repl.add_flag("--server", opts.repl_server, "Start an nREPL server");

  cli.require_subcommand(1);
  cli.failure_message(CLI::FailureMessage::help);
  CLI11_PARSE(cli, argc, argv);

  profile::configure(opts);
  profile::timer timer{ "main" };

  runtime::context rt_ctx{ opts };

  if(cli.got_subcommand(&cli_run))
  { run(opts, rt_ctx); }
  else if(cli.got_subcommand(&cli_compile))
  { compile(opts, rt_ctx); }
  else if(cli.got_subcommand(&cli_repl))
  { repl(opts, rt_ctx); }
}
/* TODO: Unify error handling. JEEZE! */
catch(std::exception const &e)
{ fmt::println("Exception: {}", e.what()); }
catch(jank::runtime::object_ptr const o)
{ fmt::println("Exception: {}", jank::runtime::detail::to_string(o)); }
catch(jank::native_string const &s)
{ fmt::println("Exception: {}", s); }
catch(jank::read::error const &e)
{ fmt::println("Read error: {}", e.message); }
