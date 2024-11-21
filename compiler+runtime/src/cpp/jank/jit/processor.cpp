#include <cstdlib>

#include <clang/AST/Type.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendDiagnostic.h>
#include <llvm/Support/Signals.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>

#include <fmt/ranges.h>

#include <jank/util/process_location.hpp>
#include <jank/util/make_array.hpp>
#include <jank/util/dir.hpp>
#include <jank/jit/processor.hpp>
#include <jank/profile/time.hpp>

namespace jank::jit
{
  static void handle_fatal_llvm_error(void * const user_data,
                                      char const *message,
                                      native_bool const gen_crash_diag)
  {
    auto &diags(*static_cast<clang::DiagnosticsEngine *>(user_data));
    diags.Report(clang::diag::err_fe_error_backend) << message;

    /* Run the interrupt handlers to make sure any special cleanups get done, in
       particular that we remove files registered with RemoveFileOnSignal. */
    llvm::sys::RunInterruptHandlers();

    /* We cannot recover from llvm errors.  When reporting a fatal error, exit
       with status 70 to generate crash diagnostics.  For BSD systems this is
       defined as an internal software error. Otherwise, exit with status 1. */
    std::exit(gen_crash_diag ? 70 : 1);
  }

  processor::processor(native_integer const optimization_level)
    : optimization_level{ optimization_level }
  {
    profile::timer timer{ "jit ctor" };
    /* TODO: Pass this into each fn below so we only do this once on startup. */
    auto const jank_path(util::process_location().unwrap().parent_path());
    auto const include_path(jank_path / "../include");

    native_persistent_string O{ "-O0" };
    switch(optimization_level)
    {
      case 0:
        break;
      case 1:
        O = "-O1";
        break;
      case 2:
        O = "-O2";
        break;
      case 3:
        O = "-Ofast";
        break;
      default:
        throw std::runtime_error{ fmt::format("invalid optimization level {}",
                                              optimization_level) };
    }

    /* When we AOT compile the jank compiler/runtime, we keep track of the compiler
     * flags used so we can use the same set during JIT compilation. Here we parse these
     * into a vector for Clang. Since Clang wants a vector<char const*>, we need to
     * dynamically allocate. These will never be freed. */
    std::vector<char const *> args{};
    std::stringstream flags{ JANK_JIT_FLAGS };
    std::string flag;
    while(std::getline(flags, flag, ' '))
    {
      args.emplace_back(strdup(flag.c_str()));
    }
    args.emplace_back(strdup(O.c_str()));

    //fmt::println("jit flags {}", args);

    clang::IncrementalCompilerBuilder compiler_builder;
    compiler_builder.SetCompilerArgs(args);
    auto compiler_instance(llvm::cantFail(compiler_builder.CreateCpp()));
    llvm::install_fatal_error_handler(handle_fatal_llvm_error,
                                      static_cast<void *>(&compiler_instance->getDiagnostics()));

    compiler_instance->LoadRequestedPlugins();

    interpreter = llvm::cantFail(clang::Interpreter::create(std::move(compiler_instance)));
  }

  processor::~processor()
  {
    llvm::remove_fatal_error_handler();
  }

  result<option<runtime::object_ptr>, native_persistent_string>
  processor::eval(codegen::processor &cg_prc) const
  {
    profile::timer timer{ "jit eval" };
    auto const str(cg_prc.declaration_str());
    //fmt::println("// declaration\n{}\n", str);

    auto declare_res(interpreter->ParseAndExecute({ str.data(), str.size() }));
    native_bool const declare_error{ declare_res };
    llvm::logAllUnhandledErrors(std::move(declare_res), llvm::errs(), "error: ");
    if(declare_error)
    {
      return err("compilation error: declaration");
    }

    auto const expr(cg_prc.expression_str(true));
    if(expr.empty())
    {
      return ok(none);
    }

    std::string full_expr{ fmt::format("&{}->base", expr) };

    //fmt::println("// expression:\n{}\n", full_expr);

    clang::Value ret;
    auto expr_res(interpreter->ParseAndExecute(full_expr, &ret));
    native_bool const expr_error{ expr_res };
    llvm::logAllUnhandledErrors(std::move(expr_res), llvm::errs(), "error: ");
    if(expr_error)
    {
      return err("compilation error: expression");
    }

    return ret.convertTo<runtime::object *>();
  }

  void processor::eval_string(native_persistent_string const &s) const
  {
    profile::timer timer{ "jit eval_string" };
    //fmt::println("// eval_string:\n{}\n", s);
    auto err(interpreter->ParseAndExecute({ s.data(), s.size() }));
    llvm::logAllUnhandledErrors(std::move(err), llvm::errs(), "error: ");
  }
}
