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

#include <folly/FBString.h>

#include <jank/util/mapped_file.hpp>
#include <jank/read/lex.hpp>
#include <jank/read/parse.hpp>
#include <jank/runtime/context.hpp>
#include <jank/analyze/processor.hpp>
#include <jank/codegen/processor.hpp>
#include <jank/evaluate.hpp>
#include <jank/jit/processor.hpp>
#include <jank/native_persistent_string.hpp>

namespace jank
{
  void run(util::cli::options const &opts, runtime::context &rt_ctx)
  {
    {
      profile::timer timer{ "require clojure.core" };
      rt_ctx.load_module("/clojure.core").expect_ok();
    }

    {
      profile::timer timer{ "eval user code" };
      std::cout << runtime::detail::to_string(rt_ctx.eval_file(opts.target_file)) << std::endl;
    }

    //ankerl::nanobench::Config config;
    //config.mMinEpochIterations = 1000000;
    //config.mOut = &std::cout;
    //config.mWarmup = 10000;


    //ankerl::nanobench::Bench().config(config).run
    //(
    //  "thing",
    //  [&]
    //  {
    //    auto const ret();
    //    ankerl::nanobench::doNotOptimizeAway(ret);
    //  }
    //);
  }

  void run_main(util::cli::options const &opts, runtime::context &rt_ctx)
  {
    {
      profile::timer timer{ "require clojure.core" };
      rt_ctx.load_module("/clojure.core").expect_ok();
    }

    {
      profile::timer timer{ "eval user code" };
      rt_ctx.load_module("/" + opts.target_module).expect_ok();

      auto const main_var(rt_ctx.find_var(opts.target_module, "-main").unwrap_or(nullptr));
      if(main_var)
      {
        /* TODO: Handle the case when `-main` accepts no arg. */
        runtime::detail::native_transient_vector extra_args;
        for(auto const &s : opts.extra_opts)
        {
          extra_args.push_back(make_box<runtime::obj::persistent_string>(s));
        }
        runtime::apply_to(main_var->deref(),
                          make_box<runtime::obj::vector>(extra_args.persistent()));
      }
      else
      {
        throw std::runtime_error{ fmt::format("Could not find #'{}/-main function!",
                                              opts.target_module) };
      }
    }
  }

  void compile(util::cli::options const &opts, runtime::context &rt_ctx)
  {
    //rt_ctx.load_module("/clojure.core").expect_ok();
    rt_ctx.compile_module(opts.target_ns).expect_ok();
  }

  void repl(util::cli::options const &opts, runtime::context &rt_ctx)
  {
    /* TODO: REPL server. */
    if(opts.repl_server)
    {
      throw std::runtime_error{ "Not yet implemented: REPL server" };
    }

    {
      profile::timer timer{ "require clojure.core" };
      rt_ctx.load_module("/clojure.core").expect_ok();
    }

    /* By default, RL will do tab completion for files. We disable that here. */
    rl_bind_key('\t', rl_insert);

    /* TODO: Completion. */
    /* TODO: Syntax highlighting. */
    /* TODO: Multi-line input. */
    while(auto const buf = readline("> "))
    {
      native_transient_string line{ buf };
      boost::trim(line);
      if(line.empty())
      {
        continue;
      }

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
      {
        fmt::println("Exception: {}", e.what());
      }
      catch(jank::runtime::object_ptr const o)
      {
        fmt::println("Exception: {}", jank::runtime::detail::to_string(o));
      }
      catch(jank::native_persistent_string const &s)
      {
        fmt::println("Exception: {}", s);
      }
      catch(jank::read::error const &e)
      {
        fmt::println("Read error: {}", e.message);
      }
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

  auto const parse_result(util::cli::parse(argc, argv));
  if(parse_result.is_err())
  {
    return parse_result.expect_err();
  }
  auto const &opts(parse_result.expect_ok());

  if(opts.gc_incremental)
  {
    GC_enable_incremental();
  }

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
    case util::cli::command::run_main:
      run_main(opts, rt_ctx);
      break;
  }
}
/* TODO: Unify error handling. JEEZE! */
catch(std::exception const &e)
{
  fmt::println("Exception: {}", e.what());
}
catch(jank::runtime::object_ptr const o)
{
  fmt::println("Exception: {}", jank::runtime::detail::to_string(o));
}
catch(jank::native_persistent_string const &s)
{
  fmt::println("Exception: {}", s);
}
catch(jank::read::error const &e)
{
  fmt::println("Read error: {}", e.message);
}
catch(...)
{
  fmt::println("Unknown exception thrown");
}
