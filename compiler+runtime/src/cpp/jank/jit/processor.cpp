#include <cstdlib>

#include <clang/AST/Type.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendDiagnostic.h>
#include <llvm/Support/Signals.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>

#include <jank/util/process_location.hpp>
#include <jank/util/make_array.hpp>
#include <jank/jit/processor.hpp>

namespace jank::jit
{
  static void LLVMErrorHandler(void *UserData, char const *Message, bool GenCrashDiag)
  {
    auto &Diags = *static_cast<clang::DiagnosticsEngine *>(UserData);

    Diags.Report(clang::diag::err_fe_error_backend) << Message;

    // Run the interrupt handlers to make sure any special cleanups get done, in
    // particular that we remove files registered with RemoveFileOnSignal.
    llvm::sys::RunInterruptHandlers();

    // We cannot recover from llvm errors.  When reporting a fatal error, exit
    // with status 70 to generate crash diagnostics.  For BSD systems this is
    // defined as an internal software error. Otherwise, exit with status 1.

    exit(GenCrashDiag ? 70 : 1);
  }

  option<boost::filesystem::path> find_pch()
  {
    auto const jank_path(jank::util::process_location().unwrap().parent_path());

    auto dev_path(jank_path / "incremental.pch");
    if(boost::filesystem::exists(dev_path))
    {
      return std::move(dev_path);
    }

    auto installed_path(jank_path / "../include/cpp/jank/incremental.pch");
    if(boost::filesystem::exists(installed_path))
    {
      return std::move(installed_path);
    }

    return none;
  }

  option<boost::filesystem::path> build_pch()
  {
    auto const jank_path(jank::util::process_location().unwrap().parent_path());
    auto const script_path(jank_path / "build-pch");
    auto const include_path(jank_path / "../include");
    auto const command(script_path.string() + " " + include_path.string() + " "
                       + std::string{ JANK_COMPILER_FLAGS });

    std::cerr << "Note: Looks like your first run. Building pre-compiled header… " << std::flush;

    if(std::system(command.c_str()) != 0)
    {
      std::cerr << "failed to build using this script: " << script_path << "\n";
      return none;
    }

    std::cerr << "done!\n";
    return jank_path / "../include/cpp/jank/prelude.hpp.pch";
  }

  processor::processor(native_integer const optimization_level)
    : optimization_level{ optimization_level }
  {
    profile::timer timer{ "jit ctor" };
    /* TODO: Pass this into each fn below so we only do this once on startup. */
    auto const jank_path(jank::util::process_location().unwrap().parent_path());

    auto pch_path(find_pch());
    if(pch_path.is_none())
    {
      pch_path = build_pch();

      /* TODO: Better error handling. */
      if(pch_path.is_none())
      {
        throw std::runtime_error{ "unable to find and also unable to build PCH" };
      }
    }
    auto const &pch_path_str(pch_path.unwrap().string());

    auto const include_path(jank_path / "../include");

    /* TODO: Default based on debug/release build. */
    native_persistent_string_view O{ "-O0" };
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
    std::stringstream flags{ JANK_COMPILER_FLAGS };
    std::string flag;
    while(std::getline(flags, flag, ' '))
    {
      args.emplace_back(strdup(flag.c_str()));
    }
    /* We can override the optimization level. */
    args.emplace_back(strdup(O.data()));
    /* We need to include our special incremental PCH. */
    args.emplace_back("-include-pch");
    args.emplace_back(strdup(pch_path_str.c_str()));

    fmt::println("compiler flags {}", JANK_COMPILER_FLAGS);

    clang::IncrementalCompilerBuilder CB;
    CB.SetCompilerArgs(args);
    auto CI = llvm::cantFail(CB.CreateCpp());
    llvm::install_fatal_error_handler(LLVMErrorHandler, static_cast<void *>(&CI->getDiagnostics()));

    CI->LoadRequestedPlugins();

    interpreter = llvm::cantFail(clang::Interpreter::create(std::move(CI)));
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
    jank::profile::timer timer{ "jit eval_string" };
    //fmt::println("// eval_string:\n{}\n", s);
    auto err(interpreter->ParseAndExecute({ s.data(), s.size() }));
    llvm::logAllUnhandledErrors(std::move(err), llvm::errs(), "error: ");
  }
}
