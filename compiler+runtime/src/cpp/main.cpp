#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>

#include <unistd.h>

#include <isocline.h>

#include <CppInterOp/Compatibility.h>
#include <CppInterOp/CppInterOp.h>

#include <jank/read/lex.hpp>
#include <jank/read/parse.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/core/call.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/runtime/obj/persistent_hash_map.hpp>
#include <jank/runtime/obj/persistent_string.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>
#include <jank/runtime/obj/persistent_vector_sequence.hpp>
#include <jank/runtime/detail/type.hpp>
#include <jank/analyze/processor.hpp>
#include <jank/c_api.h>
#include <jank/jit/processor.hpp>
#include <jank/aot/processor.hpp>
#include <jank/profile/time.hpp>
#include <jank/util/scope_exit.hpp>
#include <jank/util/string.hpp>
#include <jank/util/fmt/print.hpp>
#include <jank/util/try.hpp>
#include <jank/util/environment.hpp>
#include <jank/error/report.hpp>
#include <jank/environment/check_health.hpp>
#include <jank/runtime/convert/builtin.hpp>
#include <jank/terminal_repl_client.hpp>

#include <jank/compiler_native.hpp>
#include <clojure/core_native.hpp>
#include <clojure/string_native.hpp>

#ifdef JANK_PHASE_2
extern "C" void jank_load_clojure_core();
extern "C" void jank_load_clojure_string();
extern "C" void jank_load_clojure_walk();
extern "C" void jank_load_jank_nrepl_server_core();
extern "C" void jank_load_jank_nrepl_server_inspect();
extern "C" void jank_load_jank_nrepl_server_handler();
extern "C" void jank_load_jank_nrepl_server_bencode();
extern "C" void jank_load_jank_nrepl_server_capture();
extern "C" void jank_load_jank_nrepl_server_util();
extern "C" void jank_load_jank_nrepl_server_eval();
extern "C" void jank_load_jank_nrepl_server_parsec();
extern "C" void jank_load_jank_nrepl_server_handler_close();
extern "C" void jank_load_jank_nrepl_server_handler_clone();
extern "C" void jank_load_jank_nrepl_server_handler_describe();
extern "C" void jank_load_jank_nrepl_server_handler_completions();
extern "C" void jank_load_jank_nrepl_server_handler_eval();
extern "C" void jank_load_jank_nrepl_server_handler_lookup();
#endif

namespace jank
{
  using util::cli::opts;

  static void run()
  {
    using namespace jank;
    using namespace jank::runtime;

    {
      profile::timer const timer{ "load clojure.core" };
      __rt_ctx->load_module("clojure.core", module::origin::latest).expect_ok();
    }

    __rt_ctx->in_ns_var->deref().call(make_box<obj::symbol>("user"));
    __rt_ctx->intern_var("clojure.core", "refer")
      .expect_ok()
      .call(make_box<obj::symbol>("clojure.core"));

    {
      profile::timer const timer{ "eval user code" };
      __rt_ctx->eval_file(util::cli::opts.target_file);
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

  static void run_main()
  {
    using namespace jank;
    using namespace jank::runtime;

    {
      profile::timer const timer{ "require clojure.core" };
      __rt_ctx->load_module("clojure.core", module::origin::latest).expect_ok();
    }

    {
      profile::timer const timer{ "eval user code" };
      __rt_ctx->load_module(opts.target_module, module::origin::latest).expect_ok();

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

  static void compile_module()
  {
    using namespace jank;
    using namespace jank::runtime;

    if(opts.output_target == util::cli::compilation_target::unspecified)
    {
      if(opts.output_module_filename.empty())
      {
        opts.output_target = util::cli::compilation_target::object;
      }
      else
      {
        auto const ext{ std::filesystem::path{ opts.output_module_filename.c_str() }.extension() };
        if(ext == ".cpp")
        {
          opts.output_target = util::cli::compilation_target::cpp;
        }
        else if(ext == ".o")
        {
          opts.output_target = util::cli::compilation_target::object;
        }
        else
        {
          /* TODO: Dedicated error. */
          throw error::internal_failure(
            util::format("Unable to determine the output target type, given output file name '{}'. "
                         "If you provide a '.cpp' or '.o' extension, this can be inferred. "
                         "Otherwise, please provide the --output-target flag to specify.",
                         opts.output_module_filename));
        }
      }
    }
    else if(!opts.output_module_filename.empty())
    {
      auto const ext{ std::filesystem::path{ opts.output_module_filename.c_str() }.extension() };
      if((ext == ".cpp" && opts.output_target != util::cli::compilation_target::cpp)
         || (ext == ".o" && opts.output_target != util::cli::compilation_target::object))
      {
        error::warn(util::format("The output file name '{}' has the extension '{}', but the output "
                                 "target is '{}'. These appear to be mismatched.",
                                 opts.output_module_filename,
                                 ext.string(),
                                 util::cli::compilation_target_str(opts.output_target)));
      }
    }

    if(opts.target_module != "clojure.core")
    {
      __rt_ctx->load_module("clojure.core", module::origin::latest).expect_ok();
    }

    __rt_ctx->compile_module(opts.target_module).expect_ok();
  }

  static void compile()
  {
    using namespace jank;
    using namespace jank::runtime;

#ifdef JANK_PHASE_2
    {
      profile::timer const timer{ "require clojure.core" };
      __rt_ctx->load_module("clojure.core", module::origin::latest).expect_ok();
    }
#else
    if(opts.target_module != "clojure.core")
    {
      __rt_ctx->compile_module("clojure.core").expect_ok();
    }
#endif

    __rt_ctx->compile_module(opts.target_module).expect_ok();

    jank::aot::processor const aot_prc{};
    aot_prc.build_executable(opts.target_module).expect_ok();
  }
}

// NOLINTNEXTLINE(bugprone-exception-escape): This can only happen if we fail to report an error.
int main(int const argc, char const **argv)
{
  /* TODO: We need an init fn in libjank which sets all of this up so we don't
   * need to duplicate it between here and the tests and so that anyone embedding
   * jank doesn't need to duplicate it in their setup. */
  using namespace jank;
  using namespace jank::runtime;

  return jank_init_dynamic(
    argc,
    argv,
    /*init_default_ctx=*/false,
    nullptr,
    0,
    [](int const argc, char const **argv) {
      auto const parse_result(util::cli::parse_opts(argc, argv));
      if(parse_result.is_err())
      {
        return parse_result.expect_err();
      }

      if(jank::util::cli::opts.gc_incremental)
      {
        GC_enable_incremental();
      }

      profile::configure();
      profile::timer const timer{ "main" };

      if(util::cli::opts.command == util::cli::command::check_health)
      {
        return jank::environment::check_health() ? 0 : 1;
      }
      else if(util::cli::opts.command == util::cli::command::print_binary_version)
      {
        util::println("{}", util::binary_version());
        return 0;
      }
      else if(util::cli::opts.command == util::cli::command::print_cflags)
      {
        auto const compiler_args_res{ aot::build_compiler_args() };
        auto const &compiler_args{ compiler_args_res.expect_ok() };
        for(auto const arg : compiler_args)
        {
          util::print("{} ", arg);
        }
        for(auto const arg : aot::build_linker_args())
        {
          util::print("{} ", arg);
        }
        util::println("");
        return 0;
      }

      __rt_ctx = new(UseGC) runtime::context{};

      jank_load_clojure_core_native();

      __rt_ctx->module_loader.add_load_fn("jank.compiler-native", &jank_load_jank_compiler_native);

#ifdef JANK_PHASE_2
      __rt_ctx->module_loader.add_load_fn("clojure.core", &jank_load_clojure_core);
      __rt_ctx->module_loader.add_load_fn("clojure.string", &jank_load_clojure_string);
      __rt_ctx->module_loader.add_load_fn("clojure.walk", &jank_load_clojure_walk);
      __rt_ctx->module_loader.add_load_fn("jank.nrepl.server.core",
                                          &jank_load_jank_nrepl_server_core);
      __rt_ctx->module_loader.add_load_fn("jank.nrepl.server.inspect",
                                          &jank_load_jank_nrepl_server_inspect);
      __rt_ctx->module_loader.add_load_fn("jank.nrepl.server.handler",
                                          &jank_load_jank_nrepl_server_handler);
      __rt_ctx->module_loader.add_load_fn("jank.nrepl.server.bencode",
                                          &jank_load_jank_nrepl_server_bencode);
      __rt_ctx->module_loader.add_load_fn("jank.nrepl.server.capture",
                                          &jank_load_jank_nrepl_server_capture);
      __rt_ctx->module_loader.add_load_fn("jank.nrepl.server.util",
                                          &jank_load_jank_nrepl_server_util);
      __rt_ctx->module_loader.add_load_fn("jank.nrepl.server.eval",
                                          &jank_load_jank_nrepl_server_eval);
      __rt_ctx->module_loader.add_load_fn("jank.nrepl.server.parsec",
                                          &jank_load_jank_nrepl_server_parsec);
      __rt_ctx->module_loader.add_load_fn("jank.nrepl.server.handler.close",
                                          &jank_load_jank_nrepl_server_handler_close);
      __rt_ctx->module_loader.add_load_fn("jank.nrepl.server.handler.clone",
                                          &jank_load_jank_nrepl_server_handler_clone);
      __rt_ctx->module_loader.add_load_fn("jank.nrepl.server.handler.describe",
                                          &jank_load_jank_nrepl_server_handler_describe);
      __rt_ctx->module_loader.add_load_fn("jank.nrepl.server.handler.completions",
                                          &jank_load_jank_nrepl_server_handler_completions);
      __rt_ctx->module_loader.add_load_fn("jank.nrepl.server.handler.eval",
                                          &jank_load_jank_nrepl_server_handler_eval);
      __rt_ctx->module_loader.add_load_fn("jank.nrepl.server.handler.lookup",
                                          &jank_load_jank_nrepl_server_handler_lookup);
#endif

      Cpp::EnableDebugOutput(false);

      {
        runtime::detail::native_transient_vector extra_args;
        for(auto const &s : opts.extra_opts)
        {
          extra_args.push_back(make_box<runtime::obj::persistent_string>(s));
        }
        __rt_ctx->intern_var("clojure.core", "*command-line-args*")
          .expect_ok()
          ->bind_root(make_box<obj::persistent_vector>(extra_args.persistent())->seq());
      }

      switch(jank::util::cli::opts.command)
      {
        case util::cli::command::run:
          run();
          break;
        case util::cli::command::compile_module:
          compile_module();
          break;
        case util::cli::command::repl:
          jank::terminal_repl::repl();
          break;
        case util::cli::command::cpp_repl:
          jank::terminal_repl::cpp_repl();
          break;
        case util::cli::command::run_main:
          run_main();
          break;
        case util::cli::command::compile:
          compile();
          break;
        case util::cli::command::check_health:
        case util::cli::command::print_binary_version:
        case util::cli::command::print_cflags:
          break;
      }
      return 0;
    });
}
