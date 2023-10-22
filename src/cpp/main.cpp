#include <iostream>
#include <filesystem>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include <cling/Interpreter/Interpreter.h>
#include <cling/Interpreter/Value.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Lex/Preprocessor.h>

#include <readline/readline.h>
#include <readline/history.h>

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
  void run(util::cli::options const &opts, runtime::context &rt_ctx)
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

  void compile(util::cli::options const &opts, runtime::context &rt_ctx)
  {
    rt_ctx.compile_module(opts.target_ns);
  }

  void repl(util::cli::options const &opts, runtime::context &rt_ctx)
  {
    /* TODO: REPL server. */
    if(opts.repl_server)
    { throw std::runtime_error{ "Not yet implemented: REPL server" }; }

    {
      profile::timer timer{ "require clojure.core" };
      rt_ctx.load_module("clojure.core").expect_ok();
    }

    /* By default, RL will do tab completion for files. We disable that here. */
    rl_bind_key('\t', rl_insert);

    /* TODO: Completion. */
    /* TODO: Syntax highlighting. */
    /* TODO: Multi-line input. */
    while(auto const buf = readline("> "))
    {
      native_string line{ buf };
      boost::trim(line);
      if(line.empty())
      { continue; }

      /* TODO: Persist history to disk, á la .lein-repl-history. */
      /* History is persisted for this session only. */
      add_history(line.data());
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

  auto const opts(util::cli::parse(argc, argv).expect_ok());

  profile::configure(opts);
  profile::timer timer{ "main" };

  runtime::context rt_ctx{ opts };

  switch(opts.command)
  {
    case util::cli::command::run:
      run(opts, rt_ctx);
      break;
    case util::cli::command::compile:
      compile(opts, rt_ctx);
      break;
    case util::cli::command::repl:
      repl(opts, rt_ctx);
      break;
  }
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
