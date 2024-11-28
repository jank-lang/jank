#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include <llvm-c/Target.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/TargetSelect.h>

#include <readline/readline.h>
#include <readline/history.h>

#include <folly/FBString.h>

#include <jank/util/mapped_file.hpp>
#include <jank/read/lex.hpp>
#include <jank/read/parse.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/behavior/callable.hpp>
#include <jank/analyze/processor.hpp>
#include <jank/evaluate.hpp>
#include <jank/jit/processor.hpp>
#include <jank/native_persistent_string.hpp>
#include <jank/profile/time.hpp>

#include <jank/compiler_native.hpp>
#include <jank/perf_native.hpp>
#include <clojure/core_native.hpp>

namespace jank
{
  static void run(util::cli::options const &opts)
  {
    using namespace jank;
    using namespace jank::runtime;

    {
      profile::timer const timer{ "load clojure.core" };
      __rt_ctx->load_module("/clojure.core").expect_ok();
    }

    {
      profile::timer const timer{ "eval user code" };
      std::cout << runtime::to_code_string(__rt_ctx->eval_file(opts.target_file)) << "\n";
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

  static void run_main(util::cli::options const &opts)
  {
    using namespace jank;
    using namespace jank::runtime;

    {
      profile::timer const timer{ "require clojure.core" };
      __rt_ctx->load_module("/clojure.core").expect_ok();
    }

    {
      profile::timer const timer{ "eval user code" };
      __rt_ctx->load_module("/" + opts.target_module).expect_ok();

      auto const main_var(__rt_ctx->find_var(opts.target_module, "-main").unwrap_or(nullptr));
      if(main_var)
      {
        /* TODO: Handle the case when `-main` accepts no arg. */
        runtime::detail::native_transient_vector extra_args;
        for(auto const &s : opts.extra_opts)
        {
          extra_args.push_back(make_box<runtime::obj::persistent_string>(s));
        }
        runtime::apply_to(main_var->deref(),
                          make_box<runtime::obj::persistent_vector>(extra_args.persistent()));
      }
      else
      {
        throw std::runtime_error{ fmt::format("Could not find #'{}/-main function!",
                                              opts.target_module) };
      }
    }
  }

  static void compile(util::cli::options const &opts)
  {
    using namespace jank;
    using namespace jank::runtime;

    __rt_ctx->compile_module(opts.target_ns).expect_ok();
  }

  static void repl(util::cli::options const &opts)
  {
    using namespace jank;
    using namespace jank::runtime;

    /* TODO: REPL server. */
    if(opts.repl_server)
    {
      throw std::runtime_error{ "Not yet implemented: REPL server" };
    }

    {
      profile::timer const timer{ "require clojure.core" };
      __rt_ctx->load_module("/clojure.core").expect_ok();
    }

    if(!opts.target_module.empty())
    {
      profile::timer const timer{ "load main" };
      __rt_ctx->load_module("/" + opts.target_module).expect_ok();
      dynamic_call(__rt_ctx->in_ns_var->deref(), make_box<obj::symbol>(opts.target_module));
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
        auto const res(__rt_ctx->eval_string(line));
        fmt::println("{}", runtime::to_code_string(res));
      }
      /* TODO: Unify error handling. JEEZE! */
      catch(std::exception const &e)
      {
        fmt::println("Exception: {}", e.what());
      }
      catch(jank::runtime::object_ptr const o)
      {
        fmt::println("Exception: {}", jank::runtime::to_code_string(o));
      }
      catch(jank::native_persistent_string const &s)
      {
        fmt::println("Exception: {}", s);
      }
      catch(jank::read::error const &e)
      {
        fmt::println("Read error ({} - {}): {}", e.start, e.end, e.message);
      }
    }
  }

  static void cpp_repl(util::cli::options const &opts)
  {
    using namespace jank;
    using namespace jank::runtime;

    {
      profile::timer const timer{ "require clojure.core" };
      __rt_ctx->load_module("/clojure.core").expect_ok();
    }

    if(!opts.target_module.empty())
    {
      profile::timer const timer{ "load main" };
      __rt_ctx->load_module("/" + opts.target_module).expect_ok();
      dynamic_call(__rt_ctx->in_ns_var->deref(), make_box<obj::symbol>(opts.target_module));
    }

    /* By default, RL will do tab completion for files. We disable that here. */
    rl_bind_key('\t', rl_insert);

    while(auto const buf = readline("native> "))
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
        __rt_ctx->jit_prc.eval_string(line);
      }
      /* TODO: Unify error handling. JEEZE! */
      catch(std::exception const &e)
      {
        fmt::println("Exception: {}", e.what());
      }
      catch(jank::runtime::object_ptr const o)
      {
        fmt::println("Exception: {}", jank::runtime::to_code_string(o));
      }
      catch(jank::native_persistent_string const &s)
      {
        fmt::println("Exception: {}", s);
      }
      catch(jank::read::error const &e)
      {
        fmt::println("Read error ({} - {}): {}", e.start, e.end, e.message);
      }
    }
  }
}

// NOLINTNEXTLINE(bugprone-exception-escape): This can only happen if we fail to report an error.
int main(int const argc, char const **argv)
try
{
  using namespace jank;
  using namespace jank::runtime;

  /* The GC needs to enabled even before arg parsing, since our native types,
   * like strings, use the GC for allocations. It can still be configured later. */
  GC_set_all_interior_pointers(1);
  GC_enable();

  llvm::llvm_shutdown_obj Y{};

  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmParser();
  llvm::InitializeNativeTargetAsmPrinter();

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
  profile::timer const timer{ "main" };

  __rt_ctx = new(GC) runtime::context{ opts };

  jank_load_clojure_core_native();
  jank_load_jank_compiler_native();
  jank_load_jank_perf_native();

  switch(opts.command)
  {
    case util::cli::command::run:
      run(opts);
      break;
    case util::cli::command::compile:
      compile(opts);
      break;
    case util::cli::command::repl:
      repl(opts);
      break;
    case util::cli::command::cpp_repl:
      cpp_repl(opts);
      break;
    case util::cli::command::run_main:
      run_main(opts);
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
  fmt::println("Exception: {}", jank::runtime::to_code_string(o));
}
catch(jank::native_persistent_string const &s)
{
  fmt::println("Exception: {}", s);
}
catch(jank::read::error const &e)
{
  fmt::println("Read error ({} - {}): {}", e.start, e.end, e.message);
}
catch(...)
{
  fmt::println("Unknown exception thrown");
}
