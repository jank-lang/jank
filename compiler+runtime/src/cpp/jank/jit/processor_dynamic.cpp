#include <clang/AST/Type.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendDiagnostic.h>
#include <llvm/ExecutionEngine/Orc/Core.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/Debugging/DebuggerSupport.h>
#include <llvm/ExecutionEngine/Orc/Debugging/PerfSupportPlugin.h>
#include <llvm/ExecutionEngine/Orc/Debugging/DebugInfoSupport.h>
#include <llvm/ExecutionEngine/Orc/ObjectLinkingLayer.h>
#include <llvm/ExecutionEngine/Orc/TargetProcess/JITLoaderPerf.h>
#include <llvm/ExecutionEngine/JITEventListener.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Object/ObjectFile.h>
#include <llvm/Object/SymbolSize.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/Signals.h>

#include <cstdlib>
#include <filesystem>

#include <CppInterOp/Compatibility.h>
#include <CppInterOp/CppInterOp.h>

#include <jank/jit/object.hpp>
#include <jank/jit/parse_ld_script.hpp>
#include <jank/jit/processor.hpp>
#include <jank/util/make_array.hpp>
#include <jank/util/environment.hpp>
#include <jank/util/fmt/print.hpp>
#include <jank/util/clang.hpp>
#include <jank/util/clang_format.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/runtime/obj/jit_function.hpp>
#include <jank/runtime/obj/jit_variadic_function.hpp>
#include <jank/ir/processor.hpp>
#include <jank/codegen/cpp_processor.hpp>
#include <jank/profile/time.hpp>
#include <jank/error/system.hpp>
#include <jank/error/runtime.hpp>
#include <jank/error/codegen.hpp>

namespace jank::jit
{
  static jtl::immutable_string shared_lib_name(jtl::immutable_string const &lib)
#if defined(__APPLE__)
  {
    return util::format("lib{}.dylib", lib);
  }
#elif defined(__linux__)
  {
    return util::format("lib{}.so", lib);
  }
#elif defined(JANK_WINDOWS_LIKE)
  {
    return util::format("{}.dll", lib);
  }
#endif

  static jtl::immutable_string static_lib_name(jtl::immutable_string const &lib)
#if defined(JANK_WINDOWS_LIKE)
  {
    return util::format("lib{}.lib", lib);
  }
#else
  {
    return util::format("lib{}.a", lib);
  }
#endif

  static bool is_static_lib(jtl::immutable_string const &lib)
  {
    return lib.ends_with(".a");
  }

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

  processor::processor(jtl::immutable_string const &binary_version)
  {
    profile::timer const timer{ "jit ctor" };

    for(auto const &library_dir : util::cli::opts.library_dirs)
    {
      library_dirs.emplace_back(std::filesystem::absolute(library_dir.c_str()));
    }

    library_dirs.emplace_back(util::multi_arch_lib_path().c_str());

    /* When we AOT compile the jank compiler/runtime, we keep track of the compiler
     * flags used so we can use the same set during JIT compilation. Here we parse these
     * into a vector for Clang. Since Clang wants a vector<char const*>, we need to
     * dynamically allocate. These will never be freed. */
    std::vector<char const *> args{};

    if(auto const extra{ getenv("JANK_EXTRA_FLAGS") }; extra)
    {
      std::stringstream flags{ extra };
      std::string flag;
      while(std::getline(flags, flag, ' '))
      {
        args.emplace_back(strdup(flag.c_str()));
      }
    }

    if(util::cli::opts.debug || util::cli::opts.perf_profiling_enabled)
    {
      args.emplace_back("-g");
    }

    auto const clang_path_str{ util::find_clang() };
    if(clang_path_str.is_none())
    {
      throw error::system_clang_executable_not_found();
    }
    auto const clang_dir{ std::filesystem::path{ clang_path_str.unwrap().c_str() }.parent_path() };

    /* On macOS, we've seen some nasty issues with FP_NAN, FLT_MAX, and other C stdlib defines
     * not getting picked up since Clang is defaulting to the system libc++ instead of our
     * preferred libc++. We get around that by telling Clang to not add stdandard include paths
     * and we instead add our own. Outside of macOS, we don't use libc++, so this doesn't make
     * sense to have. */
    if constexpr(jtl::current_platform == jtl::platform::macos_like)
    {
      args.emplace_back("-nostdinc++");
      args.emplace_back("-I");
      args.emplace_back(strdup((clang_dir / "../include/c++/v1").string().c_str()));
    }

    args.emplace_back("-I");
    args.emplace_back(strdup((clang_dir / "../include").string().c_str()));

    auto const clang_resource_dir{ util::find_clang_resource_dir() };
    if(clang_resource_dir.is_none())
    {
      throw error::system_failure(
        util::format("Unable to find Clang {} resource dir.", JANK_CLANG_MAJOR_VERSION));
    }
    args.emplace_back("-resource-dir");
    args.emplace_back(clang_resource_dir.unwrap().c_str());

    auto const jank_resource_dir{ util::resource_dir() };
    args.emplace_back("-I");
    args.emplace_back(strdup(util::format("{}/include", jank_resource_dir).c_str()));

    args.emplace_back("-L");
    args.emplace_back(strdup(util::format("{}/lib", jank_resource_dir).c_str()));

    /* We add the JANK_JIT_FLAGS, which come from how jank was configured with CMake,
     * after all of these others so that the include paths we add above will have
     * precedence over the system include paths found in JANK_JIT_FLAGS.
     *
     * However, we put it before -include-pch, since there are defines and such in
     * JANK_JIT_FLAGS which are needed for our PCH. */
    {
      std::stringstream flags{ JANK_JIT_FLAGS };
      std::string flag;
      while(std::getline(flags, flag, ' '))
      {
        args.emplace_back(strdup(flag.c_str()));

        if(flag.starts_with("-L"))
        {
          library_dirs.emplace_back(flag.substr(2));
        }
      }
    }

    auto const pch_build_args{ args };

    auto pch_path{ util::find_pch(binary_version) };
    if(pch_path.is_none())
    {
      auto const res{ util::build_pch(pch_build_args, binary_version) };
      if(res.is_err())
      {
        throw res.expect_err();
      }
      pch_path = res.expect_ok();
    }

    auto const &pch_path_str{ pch_path.unwrap() };
    args.emplace_back("-include-pch");
    args.emplace_back(strdup(pch_path_str.c_str()));

    args.emplace_back("-w");
    args.emplace_back("-Wno-c++11-narrowing");

    util::add_system_flags(args);

    /********* Every flag after this line is user-provided. *********/

    for(auto const &include_path : util::cli::opts.include_dirs)
    {
      args.emplace_back(strdup(util::format("-I{}", include_path).c_str()));
    }

    for(auto const &library_path : util::cli::opts.library_dirs)
    {
      args.emplace_back(strdup(util::format("-L{}", library_path).c_str()));
    }

    for(auto const &define_macro : util::cli::opts.define_macros)
    {
      args.emplace_back(strdup(util::format("-D{}", define_macro).c_str()));
    }

    for(auto const &framework : util::cli::opts.frameworks)
    {
      args.emplace_back(strdup("-framework"));
      args.emplace_back(strdup(framework.c_str()));
    }

    switch(util::cli::opts.runtime_optimization_level)
    {
      case 0:
        args.push_back(strdup("-O0"));
        break;
      case 1:
        args.push_back(strdup("-O1"));
        break;
      case 2:
        args.push_back(strdup("-O2"));
        break;
      case 3:
        args.push_back(strdup("-O3"));
        break;
      default:
        args.push_back(strdup("-O0"));
        break;
    }

    /* Remove any -g flags if we don't have debug info enabled. This will drastically cut
     * down JIT compilation time. */
    if(!util::cli::opts.debug)
    {
      args.erase(
        std::ranges::remove_if(args,
                               [](char const *arg) { return std::string_view{ arg } == "-g"; })
          .begin(),
        args.end());
    }

    //util::println("jit flags {}", args);

    /* We need to try to initialize the JIT runtime in order to know if our PCH is stale.
     * If it is, we'll try to remove it, rebuild it, and then initialize again.
     *
     * This should only happen when system headers change, such as on a system update. */
    bool pch_out_of_date{};
    auto const create_interpreter{ [&] {
      pch_out_of_date = false;
      return static_cast<CppInternal::Interpreter *>(
        Cpp::CreateInterpreter(args,
                               {},
                               vfs,
                               static_cast<int>(llvm::CodeModel::Large),
                               &pch_out_of_date));
    } };

    auto *created_interpreter{ create_interpreter() };
    if(created_interpreter == nullptr && pch_out_of_date)
    {
      auto const stale_pch_path{ std::filesystem::path{ pch_path.unwrap().c_str() } };
      std::error_code remove_error;
      std::filesystem::remove(stale_pch_path, remove_error);
      if(remove_error)
      {
        throw error::system_failure(
          util::format("The stale pre-compiled header at `{}` could not be removed, but jank needs "
                       "to rebuild a new one. The system error provided is: {}",
                       stale_pch_path.string(),
                       remove_error.message()));
      }

      auto const rebuilt_pch{ util::build_pch(pch_build_args, binary_version) };
      if(rebuilt_pch.is_err())
      {
        throw rebuilt_pch.expect_err();
      }

      created_interpreter = create_interpreter();
    }

    if(created_interpreter == nullptr)
    {
      throw error::system_failure(
        "The JIT runtime failed to initialize. Clang most likely has printed some error "
        "diagnostics. Consider running `jank check-health` and reporting this issue.");
    }

    /* We need to include our special runtime PCH. */
    interpreter.reset(created_interpreter);

    auto const ee{ interpreter->getExecutionEngine() };

    if constexpr(jtl::current_platform != jtl::platform::windows_like)
    {
      if(util::cli::opts.debug || util::cli::opts.perf_profiling_enabled)
      {
        auto &ol{ ee->getObjLinkingLayer() };
        auto &oll{ llvm::cast<llvm::orc::ObjectLinkingLayer>(ol) };

        /* LLVM's JIT debug-object plumbing is platform-specific. On Mach-O, ORC requires its
         * dedicated debugger-support setup to synthesize/register JIT debug objects at all.
         * We install that first so our own tracking plugin, which mirrors LLVM's published
         * registrations into cpptrace, runs after LLVM's registration path has had a chance to
         * populate the global JIT descriptor state. */
        if constexpr(jtl::current_platform == jtl::platform::macos_like)
        {
          llvm::cantFail(llvm::orc::enableDebuggerSupport(*ee));
        }

        if constexpr(jtl::current_platform == jtl::platform::linux_like)
        {
          oll.addPlugin(llvm::cantFail(llvm::orc::DebugInfoPreservationPlugin::Create()));
        }
      }

      jit::install_object_tracking_plugin();
    }

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
    if constexpr(jtl::current_platform != jtl::platform::windows_like)
    {
      if(util::cli::opts.perf_profiling_enabled)
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
        oll.addPlugin(std::make_unique<llvm::orc::PerfSupportPlugin>(es.getExecutorProcessControl(),
                                                                     start_addr,
                                                                     end_addr,
                                                                     impl_addr,
                                                                     true,
                                                                     true));
      }
    }

    auto const &load_result{ load_libs(util::cli::opts.libs) };
    if(load_result.is_err())
    {
      throw error::system_failure(load_result.expect_err().c_str());
    }
  }

  processor::~processor()
  {
    llvm::remove_fatal_error_handler();
  }

  runtime::object_ref processor::eval(ir::module const &module) const
  {
    auto const generated{ codegen::gen_cpp(module) };
    eval_string(generated.declaration);
    native_vector<u8> arities;
    arities.reserve(module.root_fn_expr->arities.size());
    for(auto const &arity : module.root_fn_expr->arities)
    {
      arities.emplace_back(arity.params.size());
    }

    return create_function(module.arity_flags,
                           module.name,
                           arities,
                           module.root_fn_expr->is_variadic);
  }

  runtime::object_ref processor::create_function(runtime::callable_arity_flags const flags,
                                                 jtl::immutable_string const &base_name,
                                                 native_vector<u8> const &arities,
                                                 bool const is_variadic) const
  {
    /* TODO: Clean up with template. */
    if(is_variadic)
    {
      auto const ret{ runtime::make_box<jank::runtime::obj::jit_variadic_function>(flags) };
      for(auto const arity : arities)
      {
        switch(arity)
        {
          case 0:
            ret->arity_0 = reinterpret_cast<decltype(ret->arity_0)>(
              find_symbol(util::format("{}_0", base_name)).expect_ok());
            break;
          case 1:
            ret->arity_1 = reinterpret_cast<decltype(ret->arity_1)>(
              find_symbol(util::format("{}_1", base_name)).expect_ok());
            break;
          case 2:
            ret->arity_2 = reinterpret_cast<decltype(ret->arity_2)>(
              find_symbol(util::format("{}_2", base_name)).expect_ok());
            break;
          case 3:
            ret->arity_3 = reinterpret_cast<decltype(ret->arity_3)>(
              find_symbol(util::format("{}_3", base_name)).expect_ok());
            break;
          case 4:
            ret->arity_4 = reinterpret_cast<decltype(ret->arity_4)>(
              find_symbol(util::format("{}_4", base_name)).expect_ok());
            break;
          case 5:
            ret->arity_5 = reinterpret_cast<decltype(ret->arity_5)>(
              find_symbol(util::format("{}_5", base_name)).expect_ok());
            break;
          case 6:
            ret->arity_6 = reinterpret_cast<decltype(ret->arity_6)>(
              find_symbol(util::format("{}_6", base_name)).expect_ok());
            break;
          case 7:
            ret->arity_7 = reinterpret_cast<decltype(ret->arity_7)>(
              find_symbol(util::format("{}_7", base_name)).expect_ok());
            break;
          case 8:
            ret->arity_8 = reinterpret_cast<decltype(ret->arity_8)>(
              find_symbol(util::format("{}_8", base_name)).expect_ok());
            break;
          case 9:
            ret->arity_9 = reinterpret_cast<decltype(ret->arity_9)>(
              find_symbol(util::format("{}_9", base_name)).expect_ok());
            break;
          case 10:
            ret->arity_10 = reinterpret_cast<decltype(ret->arity_10)>(
              find_symbol(util::format("{}_10", base_name)).expect_ok());
            break;
          default:
            throw error::runtime_internal_failure(util::format("Unsupported arity {}.", arity));
        }
      }

      return ret;
    }
    else
    {
      auto const ret{ runtime::make_box<jank::runtime::obj::jit_function>(flags) };
      for(auto const arity : arities)
      {
        switch(arity)
        {
          case 0:
            ret->arity_0 = reinterpret_cast<decltype(ret->arity_0)>(
              find_symbol(util::format("{}_0", base_name)).expect_ok());
            break;
          case 1:
            ret->arity_1 = reinterpret_cast<decltype(ret->arity_1)>(
              find_symbol(util::format("{}_1", base_name)).expect_ok());
            break;
          case 2:
            ret->arity_2 = reinterpret_cast<decltype(ret->arity_2)>(
              find_symbol(util::format("{}_2", base_name)).expect_ok());
            break;
          case 3:
            ret->arity_3 = reinterpret_cast<decltype(ret->arity_3)>(
              find_symbol(util::format("{}_3", base_name)).expect_ok());
            break;
          case 4:
            ret->arity_4 = reinterpret_cast<decltype(ret->arity_4)>(
              find_symbol(util::format("{}_4", base_name)).expect_ok());
            break;
          case 5:
            ret->arity_5 = reinterpret_cast<decltype(ret->arity_5)>(
              find_symbol(util::format("{}_5", base_name)).expect_ok());
            break;
          case 6:
            ret->arity_6 = reinterpret_cast<decltype(ret->arity_6)>(
              find_symbol(util::format("{}_6", base_name)).expect_ok());
            break;
          case 7:
            ret->arity_7 = reinterpret_cast<decltype(ret->arity_7)>(
              find_symbol(util::format("{}_7", base_name)).expect_ok());
            break;
          case 8:
            ret->arity_8 = reinterpret_cast<decltype(ret->arity_8)>(
              find_symbol(util::format("{}_8", base_name)).expect_ok());
            break;
          case 9:
            ret->arity_9 = reinterpret_cast<decltype(ret->arity_9)>(
              find_symbol(util::format("{}_9", base_name)).expect_ok());
            break;
          case 10:
            ret->arity_10 = reinterpret_cast<decltype(ret->arity_10)>(
              find_symbol(util::format("{}_10", base_name)).expect_ok());
            break;
          default:
            throw error::runtime_internal_failure(util::format("Unsupported arity {}.", arity));
        }
      }

      return ret;
    }
  }

  void processor::eval_string(jtl::immutable_string const &s) const
  {
    eval_string(s, nullptr);
  }

  void processor::eval_string(jtl::immutable_string const &s, clang::Value * const ret) const
  {
    profile::timer const timer{ "jit eval_string" };
    auto formatted{ s };

    jtl::immutable_string_view const print_settings{ getenv("JANK_PRINT_CODEGEN") ?: "" };
    if(print_settings == "1")
    {
      formatted = util::format_cpp_source(s).expect_ok();
      util::println("\n{}\n", formatted);
    }
    auto err(interpreter->ParseAndExecute({ formatted.data(), formatted.size() }, ret));
    if(err)
    {
      llvm::logAllUnhandledErrors(jtl::move(err), llvm::errs(), "error: ");
      throw error::codegen_internal_failure("Unable to compile C++ source.");
    }
  }

  void processor::load_object(jtl::immutable_string_view const &path) const
  {
    auto const ee{ interpreter->getExecutionEngine() };
    auto file{ llvm::MemoryBuffer::getFile(std::string_view{ path }) };
    if(!file)
    {
      throw std::runtime_error{ util::format("failed to load object file: {}", path) };
    }

    auto object{ llvm::object::ObjectFile::createObjectFile(file.get()->getMemBufferRef()) };
    if(!object)
    {
      throw std::runtime_error{ util::format("failed to parse object file: {}", path) };
    }

    std::string const path_string{ path.data(), path.size() };
    auto const object_path{ std::filesystem::absolute(path_string).string() };
    /* Give each loaded object file its own resource tracker so later JITLink callbacks can tell
     * us which original `.o` file a materialized symbol came from. */
    auto const resource_tracker{ ee->getMainJITDylib().createResourceTracker() };
    jit::loaded_object object_info{ object_path, {} };
    auto const section_end{ object->get()->section_end() };
    /* Snapshot the object file's executable symbol table up front. This remains in object-file
     * address space; runtime addresses are filled in later by the ORC plugin. */
    for(auto const &[symbol, symbol_size] : llvm::object::computeSymbolSizes(*object->get()))
    {
      auto flags{ symbol.getFlags() };
      if(!flags)
      {
        continue;
      }
      if((*flags & llvm::object::SymbolRef::SF_Executable) == 0
         || (*flags & llvm::object::SymbolRef::SF_Undefined) != 0)
      {
        auto section{ symbol.getSection() };
        if(!section || *section == section_end || !(*section)->isText()
           || (*flags & llvm::object::SymbolRef::SF_Undefined) != 0)
        {
          continue;
        }
      }

      auto name{ symbol.getName() };
      auto address{ symbol.getAddress() };
      if(!name || !address || name->empty())
      {
        continue;
      }

      object_info.symbols[name->str()] = loaded_object_symbol{ *address, symbol_size };
    }
    if(auto err{
         resource_tracker->withResourceKeyDo([&](llvm::orc::ResourceKey const key) -> llvm::Error {
           register_loaded_object(key, jtl::move(object_info));
           return llvm::Error::success();
         }) })
    {
      llvm::consumeError(jtl::move(err));
      throw std::runtime_error{ util::format("failed to track object file resources: {}", path) };
    }

    /* XXX: Object files won't be able to use global ctors until jank is on the ORC
     * runtime, which likely won't happen until clang::Interpreter is on the ORC runtime. */
    /* TODO: Return result on failure. */
    llvm::cantFail(ee->addObjectFile(resource_tracker, std::move(file.get())));
  }

  void processor::load_ir_module(llvm::orc::ThreadSafeModule &&m) const
  {
    auto const &module_name{ m.getModuleUnlocked()->getName() };
    profile::timer const timer{ util::format(
      "jit ir module {}",
      jtl::immutable_string_view{ module_name.data(), module_name.size() }) };
    //m->print(llvm::outs(), nullptr);

    auto const ee(interpreter->getExecutionEngine());
    llvm::cantFail(ee->addIRModule(jtl::move(m)));
    llvm::cantFail(ee->initialize(ee->getMainJITDylib()));
  }

  void processor::load_bitcode(jtl::immutable_string const &module,
                               jtl::immutable_string_view const &bitcode) const
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
    load_ir_module({ std::move(ir_module), std::move(ctx) });
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

  jtl::option<jtl::immutable_string> processor::find_lib(jtl::immutable_string const &lib) const
  {
    std::filesystem::path const lib_path{ lib.c_str() };
    if(lib_path.is_absolute())
    {
      if(!std::filesystem::is_regular_file(lib_path))
      {
        return none;
      }
      return lib_path.string();

      return none;
    }

    if(lib.starts_with("./"))
    {
      if(std::filesystem::is_regular_file(lib.c_str()))
      {
        return std::filesystem::absolute(lib.c_str()).string();
      }
      return none;
    }

    for(auto const &lib_dir : library_dirs)
    {
      auto lib_abs_path{ util::format("{}/{}", lib_dir.string(), lib) };
      if(std::filesystem::is_regular_file(lib_abs_path.c_str()))
      {
        return lib_abs_path;
      }
    }

    return none;
  }

  jtl::result<void, jtl::immutable_string>
  processor::load_libs(native_vector<jtl::immutable_string> const &libs) const
  {
    for(auto const &lib : libs)
    {
      /* Try finding the lib literally, in case it contains a file name or a path.
       * Example: -llibfoo.a or -l./libfoo.so or -l/path/to/libfoo.a */
      {
        auto const result{ processor::find_lib(lib) };
        if(result.is_some())
        {
          auto const &found_lib{ result.unwrap() };
          if(is_static_lib(found_lib))
          {
            load_static_library(found_lib);
          }
          else
          {
            load_dynamic_library(found_lib);
          }
          continue;
        }
      }

      /* If the lib starts with a colon, force static lib loading, by still try to find it
       * normally.
       * Example: -l:foo or -l:libfoo.a or -l:./libfoo.so or -l:/path/to/libfoo.a */
      if(lib.starts_with(':'))
      {
        auto result{ processor::find_lib(lib.substr(1)) };
        if(result.is_some())
        {
          if(!is_static_lib(result.unwrap()))
          {
            return err(
              util::format("Failed to find static library '{}'. This library is not static.",
                           lib.substr(1)));
          }
          load_static_library(result.unwrap());
          continue;
        }

        auto const &stat{ static_lib_name(lib.substr(1)) };
        result = processor::find_lib(stat);
        if(result.is_none())
        {
          return err(util::format("Failed to find static library '{}'.", lib.substr(1)));
        }
        else
        {
          if(!is_static_lib(result.unwrap()))
          {
            return err(
              util::format("Failed to find static library '{}'. This library is not static.",
                           lib.substr(1)));
          }
          load_static_library(result.unwrap());
        }
      }
      /* Otherwise, just try to find the lib as first a dynamic lib and then a static lib.
       * This is the typical case.
       * Example: -lfoo */
      else
      {
        auto const &shared{ shared_lib_name(lib) };
        auto result{ processor::find_lib(shared) };
        if(result.is_none())
        {
          auto const &stat{ static_lib_name(lib) };
          result = processor::find_lib(stat);
          if(result.is_none())
          {
            return err(util::format("Failed to find library '{}'.", lib));
          }
          else
          {
            load_static_library(result.unwrap());
          }
        }
        else
        {
          load_dynamic_library(result.unwrap());
        }
      }
    }

    return ok();
  }

  namespace
  {
    constexpr usize max_ld_script_depth{ 8 };

    /* On Linux, shared libraries (.so files) can actually be text files which are ld scripts
     * teling the linker which libs to bring in. The LLVM JIT doesn't handle these, so we do
     * our own handling here. This is recursive, since the referenced .so in a ld script
     * may be a ld script itself. We don't want to get stuck in a loop with this, though. */
    void load_dynamic_library_impl(processor const &prc,
                                   jtl::immutable_string const &path,
                                   usize const depth)
    {
      if constexpr(jtl::current_platform == jtl::platform::linux_like)
      {
        if(!is_elf_file(path))
        {
          if(auto const resolved{ parse_ld_script(path) }; resolved.is_some())
          {
            if(depth >= max_ld_script_depth)
            {
              throw std::runtime_error{ util::format(
                "Exceeded the maximum GNU ld script nesting depth while loading `{}`.",
                path) };
            }

            auto const script_path{ std::filesystem::path{ path.c_str() } };
            auto resolved_path{ std::filesystem::path{ resolved.unwrap().c_str() } };
            if(resolved_path.is_relative())
            {
              resolved_path = script_path.parent_path() / resolved_path;
            }
            resolved_path = resolved_path.lexically_normal();

            if(resolved_path == script_path.lexically_normal())
            {
              throw std::runtime_error{ util::format("This GNU ld script `{}` refers to itself.",
                                                     path) };
            }

            load_dynamic_library_impl(prc,
                                      jtl::immutable_string{ resolved_path.string() },
                                      depth + 1);
            return;
          }
        }
      }

      llvm::cantFail(
        static_cast<clang::Interpreter &>(*prc.interpreter).LoadDynamicLibrary(path.data()));
    }
  }

  void processor::load_dynamic_library(jtl::immutable_string const &path) const
  {
    load_dynamic_library_impl(*this, path, 0);
  }

  void processor::load_static_library(jtl::immutable_string const &path) const
  {
    auto const ee{ interpreter->getExecutionEngine() };
    llvm::cantFail(ee->linkStaticLibraryInto(ee->getMainJITDylib(), path.c_str()));
  }
}
