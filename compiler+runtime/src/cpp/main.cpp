#include <filesystem>
#include <fstream>

#include <llvm/LineEditor/LineEditor.h>

#include <Interpreter/Compatibility.h>
#include <clang/Interpreter/CppInterOp.h>

#include <jank/read/lex.hpp>
#include <jank/read/parse.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/behavior/callable.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/runtime/obj/persistent_hash_map.hpp>
#include <jank/runtime/obj/persistent_string.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>
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
#include <jank/error/report.hpp>
#include <jank/environment/check_health.hpp>
#include <jank/runtime/convert/builtin.hpp>

#include <jank/compiler_native.hpp>
#include <jank/perf_native.hpp>
#include <clojure/core_native.hpp>
#include <clojure/string_native.hpp>

#ifdef JANK_PHASE_2
extern "C" void jank_load_clojure_core();
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
      __rt_ctx->load_module("/clojure.core", module::origin::latest).expect_ok();
    }

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
        auto const ext{ std::filesystem::path{ opts.output_module_filename }.extension() };
        if(ext == ".ll")
        {
          opts.output_target = util::cli::compilation_target::llvm_ir;
        }
        else if(ext == ".cpp")
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
                         "If you provide a '.ll', '.cpp', or '.o' extension, this can be inferred. "
                         "Otherwise, please provide the --output-type flag to specify.",
                         opts.output_module_filename));
        }
      }
    }
    else if(!opts.output_module_filename.empty())
    {
      auto const ext{ std::filesystem::path{ opts.output_module_filename }.extension() };
      if((ext == ".ll" && opts.output_target != util::cli::compilation_target::llvm_ir)
         || (ext == ".cpp" && opts.output_target != util::cli::compilation_target::cpp)
         || (ext == ".o" && opts.output_target != util::cli::compilation_target::object))
      {
        error::warn(util::format("The output file name '{}' has the extension '{}', but the output "
                                 "target is '{}'. These appear to be mismatched.",
                                 opts.output_module_filename,
                                 ext,
                                 util::cli::compilation_target_str(opts.output_target)));
      }
    }

    if(opts.output_target == util::cli::compilation_target::cpp
       && opts.codegen != util::cli::codegen_type::cpp)
    {
      /* TODO: Dedicated error. */
      throw error::internal_failure(
        util::format("Unable to output C++ when the codegen flag is set to '{}'. Please either "
                     "output a different target or change the codegen to C++.",
                     util::cli::codegen_type_str(opts.codegen)));
    }

    if(opts.target_module != "clojure.core")
    {
      __rt_ctx->load_module("/clojure.core", module::origin::latest).expect_ok();
    }
    __rt_ctx->compile_module(opts.target_module).expect_ok();
  }

  static void repl()
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

    dynamic_call(__rt_ctx->in_ns_var->deref(), make_box<obj::symbol>("user"));
    dynamic_call(__rt_ctx->intern_var("clojure.core", "refer").expect_ok(),
                 make_box<obj::symbol>("clojure.core"));

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

    auto const first_res_var{ __rt_ctx->find_var("clojure.core", "*1") };
    auto const second_res_var{ __rt_ctx->find_var("clojure.core", "*2") };
    auto const third_res_var{ __rt_ctx->find_var("clojure.core", "*3") };
    auto const error_var{ __rt_ctx->find_var("clojure.core", "*e") };

    context::binding_scope const scope{ obj::persistent_hash_map::create_unique(
      std::make_pair(first_res_var, jank_nil()),
      std::make_pair(second_res_var, jank_nil()),
      std::make_pair(third_res_var, jank_nil()),
      std::make_pair(error_var, jank_nil())) };

    /* TODO: Completion. */
    /* TODO: Syntax highlighting. */
    while(auto buf = le.readLine())
    {
      auto &line(*buf);
      util::trim(line);

      if(line.empty())
      {
        util::println("");
        continue;
      }

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

        if(res.is_some())
        {
          third_res_var->set(second_res_var->deref()).expect_ok();
          second_res_var->set(first_res_var->deref()).expect_ok();
          first_res_var->set(res.unwrap()).expect_ok();

          util::println("{}", runtime::to_code_string(res.unwrap()));
        }
      }
      JANK_CATCH(jank::util::print_exception)

      input.clear();
      util::println("");
      le.setPrompt(get_prompt("=> "));
    }
  }

  static void cpp_repl()
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

  static void compile()
  {
    using namespace jank;
    using namespace jank::runtime;

    if(opts.target_module != "clojure.core")
    {
      __rt_ctx->compile_module("clojure.core").expect_ok();
    }
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

  return jank_init(argc, argv, /*init_default_ctx=*/false, [](int const argc, char const **argv) {
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

    __rt_ctx = new(GC) runtime::context{};

    jank_load_clojure_core_native();
    jank_load_jank_compiler_native();
    jank_load_jank_perf_native();

#ifdef JANK_PHASE_2
    jank_load_clojure_core();
    __rt_ctx->module_loader.set_is_loaded("/clojure.core");
#endif

    Cpp::EnableDebugOutput(false);

    switch(jank::util::cli::opts.command)
    {
      case util::cli::command::run:
        run();
        break;
      case util::cli::command::compile_module:
        compile_module();
        break;
      case util::cli::command::repl:
        repl();
        break;
      case util::cli::command::cpp_repl:
        cpp_repl();
        break;
      case util::cli::command::run_main:
        run_main();
        break;
      case util::cli::command::compile:
        compile();
        break;
      case util::cli::command::check_health:
        break;
    }
    return 0;
  });
}
