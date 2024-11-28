#include <cstdlib>

#include <clang/AST/Type.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendDiagnostic.h>
#include <llvm/Support/Signals.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/IRReader/IRReader.h>

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
    profile::timer const timer{ "jit ctor" };
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

  void processor::eval_string(native_persistent_string const &s) const
  {
    profile::timer const timer{ "jit eval_string" };
    //fmt::println("// eval_string:\n{}\n", s);
    auto err(interpreter->ParseAndExecute({ s.data(), s.size() }));
    llvm::logAllUnhandledErrors(std::move(err), llvm::errs(), "error: ");
  }

  void processor::load_object(native_persistent_string_view const &path) const
  {
    auto &ee{ interpreter->getExecutionEngine().get() };
    auto file{ llvm::MemoryBuffer::getFile(path) };
    if(!file)
    {
      throw std::runtime_error{ fmt::format("failed to load object file: {}", path) };
    }
    /* XXX: Object files won't be able to use global ctors until jank is on the ORC
     * runtime, which likely won't happen until clang::Interpreter is on the ORC runtime. */
    /* TODO: Return result on failure. */
    llvm::cantFail(ee.addObjectFile(std::move(file.get())));
  }

  void processor::load_ir_module(std::unique_ptr<llvm::Module> m,
                                 std::unique_ptr<llvm::LLVMContext> llvm_ctx) const
  {
    profile::timer const timer{ fmt::format("jit ir module {}", m->getName()) };
    //m->print(llvm::outs(), nullptr);

    auto &ee(interpreter->getExecutionEngine().get());
    llvm::cantFail(
      ee.addIRModule(llvm::orc::ThreadSafeModule{ std::move(m), std::move(llvm_ctx) }));
    llvm::cantFail(ee.initialize(ee.getMainJITDylib()));
  }

  void processor::load_bitcode(native_persistent_string const &module,
                               native_persistent_string_view const &bitcode) const
  {
    auto ctx{ std::make_unique<llvm::LLVMContext>() };
    llvm::SMDiagnostic err{};
    llvm::MemoryBufferRef const buf{
      std::string_view{ bitcode.data(), bitcode.size() },
      module.c_str()
    };
    auto ir_module{ llvm::parseIR(buf, err, *ctx) };
    if(!ir_module)
    {
      err.print("jank", llvm::errs());
      /* TODO: Return a result. */
      throw std::runtime_error{ fmt::format("unable to load module") };
    }
    load_ir_module(std::move(ir_module), std::move(ctx));
  }
}
