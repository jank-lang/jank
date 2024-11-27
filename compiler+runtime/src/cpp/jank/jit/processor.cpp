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

namespace jank::jit
{

  native_persistent_string default_shared_lib_name(native_persistent_string const &lib)
#if defined(__APPLE__)
  {
    return fmt::format("{}.dylib", lib);
  }
#elif defined(__linux__)
  {
    return fmt::format("lib{}.so", lib);
  }
#endif

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

  option<boost::filesystem::path> find_pch()
  {
    auto const jank_path(util::process_location().unwrap().parent_path());

    auto dev_path(jank_path / "incremental.pch");
    if(boost::filesystem::exists(dev_path))
    {
      return std::move(dev_path);
    }

    auto installed_path(fmt::format("{}/incremental.pch", util::binary_cache_dir()));
    if(boost::filesystem::exists(installed_path))
    {
      return std::move(installed_path);
    }

    return none;
  }

  option<boost::filesystem::path> build_pch()
  {
    auto const jank_path(util::process_location().unwrap().parent_path());
    auto const script_path(jank_path / "build-pch");
    auto const include_path(jank_path / "../include");
    boost::filesystem::path const output_path{ fmt::format("{}/incremental.pch",
                                                           util::binary_cache_dir()) };
    boost::filesystem::create_directories(output_path.parent_path());
    auto const command(fmt::format("{} {} {} {} {}",
                                   script_path.string(),
                                   native_persistent_string_view{ JANK_CLANG_PREFIX },
                                   include_path.string(),
                                   output_path.string(),
                                   native_persistent_string_view{ JANK_JIT_FLAGS }));

    std::cerr << "Note: Looks like your first run. Building pre-compiled header… " << std::flush;

    if(std::system(command.c_str()) != 0)
    {
      std::cerr << "error!\n"
                << "Failed to build using this script: " << script_path << "\n";
      return none;
    }

    std::cerr << "done!\n";
    return output_path;
  }

  processor::processor(util::cli::options const &opts)
    : optimization_level{ opts.optimization_level }
  {
    for(auto const &library_dir : opts.library_dirs)
    {
      library_dirs.emplace_back(boost::filesystem::absolute(library_dir.c_str()));
    }

    profile::timer timer{ "jit ctor" };
    /* TODO: Pass this into each fn below so we only do this once on startup. */
    auto const jank_path(util::process_location().unwrap().parent_path());

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
    args.emplace_back(strdup(O.data()));
    /* We need to include our special incremental PCH. */
    args.emplace_back("-include-pch");
    args.emplace_back(strdup(pch_path_str.c_str()));

    for(auto const &include_path : opts.include_dirs)
    {
      args.emplace_back(strdup(fmt::format("-I{}", include_path).c_str()));
    }

    for(auto const &library_path : opts.library_dirs)
    {
      args.emplace_back(strdup(fmt::format("-L{}", library_path).c_str()));
    }

    for(auto const &define_macro : opts.define_macros)
    {
      args.emplace_back(strdup(fmt::format("-D{}", define_macro).c_str()));
    }

    // fmt::println("jit flags {}", args);

    clang::IncrementalCompilerBuilder compiler_builder;
    compiler_builder.SetCompilerArgs(args);
    auto compiler_instance(llvm::cantFail(compiler_builder.CreateCpp()));
    llvm::install_fatal_error_handler(handle_fatal_llvm_error,
                                      static_cast<void *>(&compiler_instance->getDiagnostics()));

    compiler_instance->LoadRequestedPlugins();

    interpreter = llvm::cantFail(clang::Interpreter::create(std::move(compiler_instance)));

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

  option<native_persistent_string>
  processor::find_dynamic_lib(native_persistent_string const &lib) const
  {
    auto const &default_lib_name{ default_shared_lib_name(lib) };
    for(auto const &lib_dir : library_dirs)
    {
      auto const default_lib_abs_path{ fmt::format("{}/{}", lib_dir.string(), default_lib_name) };
      if(boost::filesystem::exists(default_lib_abs_path.c_str()))
      {
        return default_lib_abs_path;
      }
      else
      {
        auto const lib_abs_path{ fmt::format("{}/{}", lib_dir.string(), lib) };
        if(boost::filesystem::exists(lib_abs_path))
        {
          return lib_abs_path;
        }
      }
    }

    return none;
  }

  result<void, native_persistent_string>
  processor::load_dynamic_libs(native_vector<native_persistent_string> const &libs) const
  {
    for(auto const &lib : libs)
    {
      if(boost::filesystem::path{ lib.c_str() }.is_absolute())
      {
        load_object(lib);
      }
      else
      {
        auto const result{ processor::find_dynamic_lib(lib) };
        if(result.is_none())
        {
          return err(fmt::format("Failed to load dynamic library `{}`", lib));
        }
        else
        {
          load_object(result.unwrap());
        }
      }
    }

    return ok();
  }

  void processor::load_object(native_persistent_string const &path) const
  {
    llvm::cantFail(interpreter->LoadDynamicLibrary(path.data()));
  }
}
