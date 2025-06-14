#include <iostream>
#include <filesystem>
#include <fstream>

#include <llvm/LineEditor/LineEditor.h>

#include <jank/aot/processor.hpp>
#include <jank/read/lex.hpp>
#include <jank/read/parse.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/behavior/callable.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/runtime/obj/persistent_string.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>
#include <jank/runtime/detail/type.hpp>
#include <jank/analyze/processor.hpp>
#include <jank/c_api.h>
#include <jank/evaluate.hpp>
#include <jank/jit/processor.hpp>
#include <jank/profile/time.hpp>
#include <jank/error/report.hpp>
#include <jank/util/scope_exit.hpp>
#include <jank/util/string.hpp>
#include <jank/util/fmt/print.hpp>
#include <jank/util/try.hpp>

#include <jank/compiler_native.hpp>
#include <jank/perf_native.hpp>
#include <clojure/core_native.hpp>
#include <clojure/string_native.hpp>

namespace jank
{
  static void run(util::cli::options const &opts)
  {
    using namespace jank;
    using namespace jank::runtime;

    {
      profile::timer const timer{ "load clojure.core" };
      __rt_ctx->load_module("/clojure.core", module::origin::latest).expect_ok();
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
      __rt_ctx->load_module("/clojure.core", module::origin::latest).expect_ok();
    }

    {
      profile::timer const timer{ "eval user code" };
      __rt_ctx->load_module("/" + opts.target_module, module::origin::latest).expect_ok();

      auto const main_var(__rt_ctx->find_var(opts.target_module, "-main"));
      if(main_var.is_some())
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
        throw std::runtime_error{ util::format("Could not find #'{}/-main function!",
                                               opts.target_module) };
      }
    }
  }

  static void compile_module(util::cli::options const &opts)
  {
    using namespace jank;
    using namespace jank::runtime;

    if(opts.target_ns != "clojure.core")
    {
      __rt_ctx->load_module("/clojure.core", module::origin::latest).expect_ok();
    }
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
      __rt_ctx->load_module("/clojure.core", module::origin::latest).expect_ok();
    }

    if(!opts.target_module.empty())
    {
      profile::timer const timer{ "load main" };
      __rt_ctx->load_module("/" + opts.target_module, module::origin::latest).expect_ok();
      dynamic_call(__rt_ctx->in_ns_var->deref(), make_box<obj::symbol>(opts.target_module));
    }

    auto const get_prompt([](jtl::immutable_string const &suffix) {
      return __rt_ctx->current_ns()->name->to_code_string() + suffix;
    });

    /* By default we are placed in clojure.core ns as of now.
     * TODO: Set default ns to `user` when we are dropped in that ns.*/
    llvm::LineEditor le("jank", ".jank-repl-history");
    le.setPrompt(get_prompt("=> "));
    native_transient_string input{};

    /* We write every REPL expression to a temporary file, which then allows us
     * to later review that for error reporting. We automatically clean it up
     * and we reuse the same file over and over. */
    auto const tmp{ std::filesystem::temp_directory_path() };
    std::string path_tmp{ tmp / "jank-repl-XXXXXX" };
    mkstemp(path_tmp.data());

    /* TODO: Completion. */
    /* TODO: Syntax highlighting. */
    while(auto buf = le.readLine())
    {
      auto &line(*buf);
      util::trim(line);

      if(line.ends_with("\\"))
      {
        input.append(line.substr(0, line.size() - 1));
        input.append("\n");
        le.setPrompt(get_prompt("=>... "));
        continue;
      }

      input += line;

      util::scope_exit const finally{ [&] { std::filesystem::remove(path_tmp); } };
      JANK_TRY
      {
        {
          std::ofstream ofs{ path_tmp };
          ofs << input;
        }

        auto const res(__rt_ctx->eval_file(path_tmp));
        util::println("{}", runtime::to_code_string(res));
      }
      JANK_CATCH(jank::util::print_exception)

      input.clear();
      std::cout << "\n";
      le.setPrompt(get_prompt("=> "));
    }
  }

  static void cpp_repl(util::cli::options const &opts)
  {
    using namespace jank;
    using namespace jank::runtime;

    {
      profile::timer const timer{ "require clojure.core" };
      __rt_ctx->load_module("/clojure.core", module::origin::latest).expect_ok();
    }

    if(!opts.target_module.empty())
    {
      profile::timer const timer{ "load main" };
      __rt_ctx->load_module("/" + opts.target_module, module::origin::latest).expect_ok();
      dynamic_call(__rt_ctx->in_ns_var->deref(), make_box<obj::symbol>(opts.target_module));
    }

    llvm::LineEditor le("jank-native", ".jank-native-repl-history");
    le.setPrompt("native> ");
    native_transient_string input{};

    while(auto buf = le.readLine())
    {
      auto &line(*buf);
      util::trim(line);

      if(line.empty())
      {
        continue;
      }

      if(line.ends_with("\\"))
      {
        input.append(line.substr(0, line.size() - 1));
        le.setPrompt("native>... ");
        continue;
      }

      input += line;

      JANK_TRY
      {
        __rt_ctx->jit_prc.eval_string(input);
      }
      JANK_CATCH(jank::util::print_exception)

      input.clear();
      le.setPrompt("native> ");
    }
  }

  static void compile(util::cli::options const &opts)
  {
    using namespace jank;
    using namespace jank::runtime;

    if(opts.target_ns != "clojure.core")
    {
      __rt_ctx->compile_module("clojure.core").expect_ok();
    }
    __rt_ctx->compile_module(opts.target_module).expect_ok();

    jank::aot::processor const aot_prc{ opts };
    auto const res{ aot_prc.compile(opts.target_module) };
    if(res.is_err())
    {
      throw std::runtime_error{ util::format("Exitted with code: {}. {}",
                                             res.expect_err().return_code,
                                             res.expect_err().err_message) };
    }
  }
}

// NOLINTNEXTLINE(bugprone-exception-escape): This can only happen if we fail to report an error.
int main(int const argc, char const **argv)
{
  using namespace jank;
  using namespace jank::runtime;

  return jank_init(argc, argv, /*init_default_ctx=*/false, [](int const argc, char const **argv) {
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
    jank_load_clojure_string_native();
    jank_load_jank_compiler_native();
    jank_load_jank_perf_native();

    switch(opts.command)
    {
      case util::cli::command::run:
        run(opts);
        break;
      case util::cli::command::compile_module:
        compile_module(opts);
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
      case util::cli::command::compile:
        compile(opts);
        break;
    }
    return 0;
  });
}
