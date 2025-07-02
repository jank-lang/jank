#include <cstdlib>

#include <clang/AST/Type.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendDiagnostic.h>
#include <Interpreter/Compatibility.h>
#include <clang/Interpreter/CppInterOp.h>
#include <llvm/ExecutionEngine/Orc/Core.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/Signals.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/Debugging/PerfSupportPlugin.h>
#include <llvm/ExecutionEngine/Orc/Debugging/DebugInfoSupport.h>
#include <llvm/ExecutionEngine/Orc/TargetProcess/JITLoaderPerf.h>
#include <llvm/ExecutionEngine/JITEventListener.h>
#include <llvm/IRReader/IRReader.h>

#include <cpptrace/gdb_jit.hpp>

#include <jank/util/process_location.hpp>
#include <jank/util/make_array.hpp>
#include <jank/util/dir.hpp>
#include <jank/util/fmt/print.hpp>
#include <jank/jit/processor.hpp>
#include <jank/profile/time.hpp>

namespace jank::jit
{
  static jtl::immutable_string default_shared_lib_name(jtl::immutable_string const &lib)
#if defined(__APPLE__)
  {
    return util::format("{}.dylib", lib);
  }
#elif defined(__linux__)
  {
    return util::format("lib{}.so", lib);
  }
#endif

  [[maybe_unused]]
  static void
  handle_fatal_llvm_error(void * const user_data, char const *message, bool const gen_crash_diag)
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

  /* LLVM will register JIT compiled frames for GDB/LLDB using a standard
   * interface which is described here:
   *
   * https://weliveindetail.github.io/blog/post/2022/11/27/gdb-jit-interface-101.html
   *
   * The debuggers implicitly place breakpoints on the `__jit_debug_register_code`
   * function, which is called as part of LLVM's registration. This is an empty
   * function, but the breakpoint triggering tells the debugger to update its
   * entries based on the `__jit_debug_descriptor` linked list.
   *
   * Here, we manually trigger the same thing, to have cpptrace update its
   * view of the available JIT compiler frames. We do this after loading any
   * new JIT compiled code. */
  static void register_jit_stack_frames()
  {
    if(auto *entry = cpptrace::detail::__jit_debug_descriptor.relevant_entry)
    {
      cpptrace::register_jit_object(entry->symfile_addr, entry->symfile_size);
    }
  }

  processor::processor(util::cli::options const &opts)
    : optimization_level{ opts.optimization_level }
  {
    profile::timer const timer{ "jit ctor" };

    for(auto const &library_dir : opts.library_dirs)
    {
      library_dirs.emplace_back(std::filesystem::absolute(library_dir.c_str()));
    }

    jtl::immutable_string O{ "-O0" };
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
        /* TODO: Use -O3? */
        O = "-Ofast";
        break;
      default:
        throw std::runtime_error{ util::format("invalid optimization level {}",
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

    if(opts.perf_profiling_enabled)
    {
      O = "-Og";
      args.emplace_back("-ggdb");
    }
    args.emplace_back(strdup(O.c_str()));

    for(auto const &include_path : opts.include_dirs)
    {
      args.emplace_back(strdup(util::format("-I{}", include_path).c_str()));
    }

    for(auto const &library_path : opts.library_dirs)
    {
      args.emplace_back(strdup(util::format("-L{}", library_path).c_str()));
    }

    for(auto const &define_macro : opts.define_macros)
    {
      args.emplace_back(strdup(util::format("-D{}", define_macro).c_str()));
    }

    /* TODO: Use the jank path to find this again. Find the old code. */
    args.emplace_back("-I/home/jeaye/projects/jank/compiler+runtime/include/cpp");
    args.emplace_back("-I/home/jeaye/projects/jank/compiler+runtime/third-party/nanobench/include");
    args.emplace_back("-I/home/jeaye/projects/jank/compiler+runtime/third-party/folly");
    args.emplace_back("-I/home/jeaye/projects/jank/compiler+runtime/third-party/bpptree/include");
    args.emplace_back("-I/home/jeaye/projects/jank/compiler+runtime/third-party/immer");
    args.emplace_back("-I/home/jeaye/projects/jank/compiler+runtime/third-party/cli11/include");
    args.emplace_back("-I/home/jeaye/projects/jank/compiler+runtime/third-party/ftxui/include");
    args.emplace_back("-I/home/jeaye/projects/jank/compiler+runtime/third-party/libzippp/src");
    args.emplace_back("-I/home/jeaye/projects/jank/compiler+runtime/third-party/cpptrace/include");
    args.emplace_back(
      "-I/home/jeaye/projects/jank/compiler+runtime/third-party/cppinterop/include");
    args.emplace_back("-I/home/jeaye/projects/jank/compiler+runtime/third-party/cppinterop/lib");
    args.emplace_back(
      "-I/home/jeaye/projects/jank/compiler+runtime/third-party/boost-preprocessor/include");
    args.emplace_back(
      "-I/home/jeaye/projects/jank/compiler+runtime/third-party/boost-multiprecision/include");

    /* TODO: Pass in clang binary name as macro define. */
    auto const resource_dir{ Cpp::DetectResourceDir("clang++") };
    args.emplace_back("-resource-dir");
    args.emplace_back(strdup(resource_dir.c_str()));

    std::vector<std::string> sys_includes;
    Cpp::DetectSystemCompilerIncludePaths(sys_includes, "clang++");
    for(auto const &i : sys_includes)
    {
      args.emplace_back(strdup(util::format("-I{}", i).c_str()));
    }

    //util::println("jit flags {}", args);

    interpreter.reset(static_cast<Cpp::Interpreter *>(Cpp::CreateInterpreter(args)));

    /* Enabling perf support requires registering a couple of plugins with LLVM. These
     * plugins will generate files which perf can then use to inject additional info
     * into its recorded data (via `perf inject`).
     *
     * Note that we need to manually get the start/end/impl address for perf, rather than
     * using the PerfSupportPlugin::Create factory, since the latter leads to crashes on
     * Clang 19, at least. This workaround was suggested by and borrowed from Julia devs.
     *
     * https://github.com/mortenpi/julia/blob/1edc6f1b7752ed67059020ba7ce174dffa225954/src/jitlayers.cpp#L2330
     */
    if(opts.perf_profiling_enabled)
    {
      auto const ee{ interpreter->getExecutionEngine() };
      auto &es{ ee->getExecutionSession() };
      auto &ol{ ee->getObjLinkingLayer() };
      auto &oll{ llvm::cast<llvm::orc::ObjectLinkingLayer>(ol) };

#define add_address_to_map(map, name)                                     \
  ((map)[es.intern(ee->mangle(#name))]                                    \
   = { llvm::orc::ExecutorAddr::fromPtr(&(name)),                         \
       llvm::JITSymbolFlags::Exported | llvm::JITSymbolFlags::Callable }, \
   llvm::orc::ExecutorAddr::fromPtr(&(name)))

      llvm::orc::SymbolMap perf_fns;
      auto const start_addr{ add_address_to_map(perf_fns, llvm_orc_registerJITLoaderPerfStart) };
      auto const end_addr{ add_address_to_map(perf_fns, llvm_orc_registerJITLoaderPerfEnd) };
      auto const impl_addr{ add_address_to_map(perf_fns, llvm_orc_registerJITLoaderPerfImpl) };
      llvm::cantFail(ee->getMainJITDylib().define(llvm::orc::absoluteSymbols(perf_fns)));
      oll.addPlugin(llvm::cantFail(llvm::orc::DebugInfoPreservationPlugin::Create()));
      oll.addPlugin(std::make_unique<llvm::orc::PerfSupportPlugin>(es.getExecutorProcessControl(),
                                                                   start_addr,
                                                                   end_addr,
                                                                   impl_addr,
                                                                   true,
                                                                   true));
    }

    auto const &load_result{ load_dynamic_libs(opts.libs) };
    if(load_result.is_err())
    {
      throw std::runtime_error{ load_result.expect_err().c_str() };
    }
  }

  processor::~processor()
  {
    llvm::remove_fatal_error_handler();
  }

  void processor::eval_string(jtl::immutable_string const &s) const
  {
    profile::timer const timer{ "jit eval_string" };
    //util::println("// eval_string:\n{}\n", s);
    auto err(interpreter->ParseAndExecute({ s.data(), s.size() }));
    llvm::logAllUnhandledErrors(std::move(err), llvm::errs(), "error: ");
  }

  void processor::load_object(native_persistent_string_view const &path) const
  {
    auto const ee{ interpreter->getExecutionEngine() };
    auto file{ llvm::MemoryBuffer::getFile(path) };
    if(!file)
    {
      throw std::runtime_error{ util::format("failed to load object file: {}", path) };
    }
    /* XXX: Object files won't be able to use global ctors until jank is on the ORC
     * runtime, which likely won't happen until clang::Interpreter is on the ORC runtime. */
    /* TODO: Return result on failure. */
    llvm::cantFail(ee->addObjectFile(std::move(file.get())));
    register_jit_stack_frames();
  }

  void processor::load_ir_module(std::unique_ptr<llvm::Module> m,
                                 std::unique_ptr<llvm::LLVMContext> llvm_ctx) const
  {
    profile::timer const timer{ util::format("jit ir module {}",
                                             static_cast<std::string_view>(m->getName())) };
    //m->print(llvm::outs(), nullptr);

    auto const ee(interpreter->getExecutionEngine());
    llvm::cantFail(
      ee->addIRModule(llvm::orc::ThreadSafeModule{ std::move(m), std::move(llvm_ctx) }));
    llvm::cantFail(ee->initialize(ee->getMainJITDylib()));
    register_jit_stack_frames();
  }

  void processor::load_bitcode(jtl::immutable_string const &module,
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
      throw std::runtime_error{ util::format("unable to load module") };
    }
    load_ir_module(std::move(ir_module), std::move(ctx));
  }

  jtl::string_result<void> processor::remove_symbol(jtl::immutable_string const &name) const
  {
    auto const ee{ interpreter->getExecutionEngine() };
    llvm::orc::SymbolNameSet to_remove{};
    to_remove.insert(ee->mangleAndIntern(name.c_str()));
    auto const error{ ee->getMainJITDylib().remove(to_remove) };

    if(error.isA<llvm::orc::SymbolsCouldNotBeRemoved>())
    {
      return err(util::format("Failed to remove the symbol: '{}'", name));
    }
    return ok();
  }

  jtl::string_result<void *> processor::find_symbol(jtl::immutable_string const &name) const
  {
    if(auto symbol{ interpreter->getSymbolAddress(name.c_str()) })
    {
      return symbol.get().toPtr<void *>();
    }

    return err(util::format("Failed to find symbol: '{}'", name));
  }

  jtl::option<jtl::immutable_string>
  processor::find_dynamic_lib(jtl::immutable_string const &lib) const
  {
    auto const &default_lib_name{ default_shared_lib_name(lib) };
    for(auto const &lib_dir : library_dirs)
    {
      auto default_lib_abs_path{ util::format("{}/{}", lib_dir.string(), default_lib_name) };
      if(std::filesystem::exists(default_lib_abs_path.c_str()))
      {
        return default_lib_abs_path;
      }
      else
      {
        auto lib_abs_path{ util::format("{}/{}", lib_dir.string(), lib) };
        if(std::filesystem::exists(lib_abs_path.c_str()))
        {
          return lib_abs_path;
        }
      }
    }

    return none;
  }

  jtl::result<void, jtl::immutable_string>
  processor::load_dynamic_libs(native_vector<jtl::immutable_string> const &libs) const
  {
    for(auto const &lib : libs)
    {
      if(std::filesystem::path{ lib.c_str() }.is_absolute())
      {
        load_dynamic_library(lib);
      }
      else
      {
        auto const result{ processor::find_dynamic_lib(lib) };
        if(result.is_none())
        {
          return err(util::format("Failed to load dynamic library `{}`", lib));
        }
        else
        {
          load_dynamic_library(result.unwrap());
        }
      }
    }

    return ok();
  }

  void processor::load_dynamic_library(jtl::immutable_string const &path) const
  {
    llvm::cantFail(static_cast<clang::Interpreter &>(*interpreter).LoadDynamicLibrary(path.data()));
  }
}
