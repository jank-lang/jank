#include <filesystem>
#include <fstream>

#include <Interpreter/Compatibility.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/DiagnosticIDs.h>
#include <clang/Basic/DiagnosticOptions.h>
#include <clang/Driver/Compilation.h>
#include <clang/Driver/Driver.h>
#include <clang/Driver/ToolChain.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>

#include <llvm/TargetParser/Host.h>
#include <llvm/Support/Program.h>
#include <llvm/Support/VirtualFileSystem.h>

#include <jank/util/clang.hpp>
#include <jank/util/dir.hpp>
#include <jank/util/fmt/print.hpp>
#include <jank/util/scope_exit.hpp>
#include <jank/runtime/context.hpp>
#include <jank/aot/resource.hpp>
#include <jank/error/system.hpp>

namespace Cpp
{
  std::string DetectResourceDir(char const *ClangBinaryName = "clang");
}

namespace jank::util
{
  static bool is_clang_correct_version(std::filesystem::path const &path)
  {
    auto const tmp{ std::filesystem::temp_directory_path() };
    std::string path_tmp{ tmp / "jank-clang-XXXXXX" };
    mkstemp(path_tmp.data());
    auto const proc_code{ llvm::sys::ExecuteAndWait(path.c_str(),
                                                    { path.c_str(), "--version" },
                                                    std::nullopt,
                                                    { std::nullopt, path_tmp, std::nullopt }) };
    if(proc_code < 0)
    {
      return false;
    }

    std::string version_line;
    std::ifstream ifs{ path_tmp };
    std::getline(ifs, version_line);
    auto const found{ version_line.find("version " JANK_CLANG_MAJOR_VERSION) };
    return found != std::string::npos;
  }

  /* Whenever we need to invoke Clang, at run time, we need to make sure we're using
   * a matching Clang to what was used to build jank. We give this multiple attempts.
   *
   * 1. jank is configured with a JANK_CLANG_PATH which says the path to the exact
   *    Clang which was used to build jank. In most cases of installing jank on
   *    someone's machine, this should work well.
   *
   * 2. When installing jank with a local Clang/LLVM build, we also install Clang
   *    in jank's resource dir. In this case, JANK_CLANG_PATH won't be correct
   *    so this will be the default case.
   *
   * 3. If neither of the above is found, before we start guessing, we check the
   *    CXX environment variable. This is a standard across any POSIX-based
   *    systems. However, if CXX is defined, we invoke the provided compiler
   *    to match the version to JANK_CLANG_MAJOR_VERSION.
   *
   * 4. jank is configured with a JANK_CLANG_MAJOR_VERSION which says the major
   *    Clang version which was used to build jank. At this point, we don't care
   *    about minor version differences. So, if we were built with Clang 20, we
   *    will look for clang++-20 on the user's PATH.
   *
   * 5. Finally, if neither of the above is found, we'll look for clang++ on the
   *    user's PATH and we'll invoke it to find out which version it is. If it
   *    matches the JANK_CLANG_MAJOR_VERSION, we're good.
   *
   * In the case of running the jank compiler, we'll almost always satisfy the
   * first attempt. However, when we use jank to AOT build a user's program
   * with a dynamic jank runtime, that program will also need to find Clang.
   * This is where the other two options become much more useful.
   */
  jtl::option<jtl::immutable_string> find_clang()
  {
    static jtl::immutable_string result;
    if(!result.empty())
    {
      return result;
    }

    std::filesystem::path const configured_path{ JANK_CLANG_PATH };
    if(std::filesystem::exists(configured_path))
    {
      return result = configured_path.c_str();
    }

    std::filesystem::path const resource_dir{ util::resource_dir().c_str() };
    std::filesystem::path const installed_path{ resource_dir / "bin/clang++" };
    if(std::filesystem::exists(installed_path))
    {
      return result = installed_path.c_str();
    }

    std::filesystem::path const cxx_path{ getenv("CXX") ?: "" };
    if(std::filesystem::exists(cxx_path) && is_clang_correct_version(cxx_path))
    {
      return result = cxx_path.c_str();
    }

    auto const versioned_path{ llvm::sys::findProgramByName("clang++-" JANK_CLANG_MAJOR_VERSION) };
    if(versioned_path)
    {
      return result = *versioned_path;
    }

    auto const unversioned_path{ llvm::sys::findProgramByName("clang++") };
    if(unversioned_path && is_clang_correct_version(*unversioned_path))
    {
      return result = unversioned_path->c_str();
    }

    return none;
  }

  /* Just like we have a configured Clang, we have a configured Clang resource dir, which
   * matches our configured Clang. When we find a different Clang, we need to invoke it with
   * -print-resource-dir in order to get the new resource dir. For some reason, the JIT
   * runtime sometimes struggles to figure this out on its own, so we always provide it. */
  jtl::option<jtl::immutable_string> find_clang_resource_dir()
  {
    static jtl::immutable_string result;
    if(!result.empty())
    {
      return result;
    }

    auto const clang_path{ find_clang() };
    if(clang_path.is_none())
    {
      return none;
    }

    /* Short circuit the process creation in the scenario where we find our
     * configured clang. We can always assume that our configuration is correct,
     * even though it may not be, simply because it's not sane to operate otherwise. So
     * we can use the configured resource dir in this case. */
    if(clang_path.unwrap() == JANK_CLANG_PATH)
    {
      return JANK_CLANG_RESOURCE_DIR;
    }

    auto resource_dir{ Cpp::DetectResourceDir(clang_path.unwrap().c_str()) };

    if(resource_dir.empty())
    {
      return none;
    }
    return resource_dir;
  }

  jtl::result<void, error_ref> invoke_clang(std::vector<char const *> args)
  {
    std::string buffer;
    llvm::raw_string_ostream diag_stream{ buffer };
    clang::DiagnosticOptions diag_opts{};
    auto * const diag_client{
      new clang::TextDiagnosticPrinter{ diag_stream, diag_opts }
    };
    clang::IntrusiveRefCntPtr<clang::DiagnosticIDs> const diag_id{ new clang::DiagnosticIDs() };
    clang::DiagnosticsEngine diags{ diag_id, diag_opts, diag_client, /*ShouldOwnClient=*/true };
    auto const vfs{ llvm::vfs::getRealFileSystem() };
    auto const &target_triple{ llvm::sys::getDefaultTargetTriple() };
    auto const clang_path_res{ find_clang() };
    if(clang_path_res.is_none())
    {
      return err(error::system_clang_executable_not_found());
    }
    auto const &clang_path{ clang_path_res.unwrap() };

    /* Building the driver doesn't actually run the commands yet. All of the flags will
     * be checked, though. */
    clang::driver::Driver driver{ clang_path.c_str(), target_triple, diags, "jank", vfs };
    driver.setCheckInputsExist(true);

    /* The first argument should be the clang executable. */
    args.insert(args.begin(), clang_path.c_str());

    if(auto const extra{ getenv("JANK_EXTRA_FLAGS") }; extra)
    {
      std::stringstream flags{ extra };
      std::string flag;
      while(std::getline(flags, flag, ' '))
      {
        args.emplace_back(flag.c_str());
      }
    }

    auto const compilation_result{ driver.BuildCompilation(args) };
    if(!compilation_result || compilation_result->containsError())
    {
      return err(error::system_failure(format("Failed to build Clang steps.\n{}", buffer)));
    }

    /* Execute the compilation jobs (preprocess, compile, assemble).
     * This actually runs the commands determined by BuildCompilation. */
    int execution_exit_code{ 1 };
    if(compilation_result && !compilation_result->containsError())
    {
      llvm::SmallVector<std::pair<int, clang::driver::Command const *>> failures;
      execution_exit_code = driver.ExecuteCompilation(*compilation_result, failures);
    }

    if(diags.hasErrorOccurred() || execution_exit_code != 0)
    {
      return err(error::system_failure(format("Clang failed with errors.\n{}", buffer)));
    }

    return ok();
  }

  jtl::option<jtl::immutable_string> find_pch(jtl::immutable_string const &binary_version)
  {
    std::filesystem::path const jank_path{ process_dir().c_str() };

    /* For AOT compiled jank programs, the PCH is embedded within the executable as a resource.
     * In this case, we just grab a pointer to the data and set it up with Clang's VFS so
     * no copying or file IO is required. */
    auto const resource{ aot::find_resource("incremental.pch") };
    if(resource.is_some())
    {
      runtime::__rt_ctx->jit_prc.vfs["/virtual/incremental.pch"]
        = { resource.unwrap().data(), resource.unwrap().size() };
      return "/virtual/incremental.pch";
    }

    auto dev_path{ jank_path / "incremental.pch" };
    if(std::filesystem::exists(dev_path))
    {
      return dev_path.c_str();
    }

    std::string const installed_path{ format("{}/incremental.pch",
                                             user_cache_dir(binary_version)) };
    if(std::filesystem::exists(installed_path))
    {
      return installed_path.c_str();
    }

    return none;
  }

  jtl::result<jtl::immutable_string, error_ref>
  build_pch(std::vector<char const *> args, jtl::immutable_string const &binary_version)
  {
    /* TODO: Remove these logs for the alpha release. */
    print(stderr,
          "Note: Looks like your first run with these flags. Building pre-compiled header… ");

    std::filesystem::path const jank_path{ process_dir().c_str() };
    auto include_path{ jank_path / "../include/cpp/jank/prelude.hpp" };
    if(!std::filesystem::exists(include_path))
    {
      auto const install_path{ util::resource_dir() + "/include/jank/prelude.hpp" };
      if(!std::filesystem::exists(install_path.c_str()))
      {
        println(stderr, "failed!");
        return err(error::system_failure(
          util::format("Unable to find PCH entrypoint. Tried these paths:\n\n{}\n{}",
                       include_path.c_str(),
                       install_path)));
      }
      include_path = install_path;
    }

    std::filesystem::path const output_path{ format("{}/incremental.pch",
                                                    user_cache_dir(binary_version)) };
    std::filesystem::create_directories(output_path.parent_path());

    args.emplace_back("-Xclang");
    args.emplace_back("-fincremental-extensions");
    args.emplace_back("-Xclang");
    args.emplace_back("-emit-pch");
    args.emplace_back("-Xclang");
    args.emplace_back("-fmodules-embed-all-files");
    args.emplace_back("-fno-modules-validate-system-headers");
    args.emplace_back("-fpch-instantiate-templates");
    args.emplace_back("-Xclang");
    args.emplace_back("-fno-validate-pch");
    args.emplace_back("-Xclang");
    args.emplace_back("-fno-pch-timestamp");
    args.emplace_back("-x");
    args.emplace_back("c++-header");
    args.emplace_back("-o");
    args.emplace_back(output_path.c_str());
    args.emplace_back("-c");
    args.emplace_back(include_path.c_str());
    /* We need to add this again for it to get through. Not sure why. */
    args.emplace_back("-std=gnu++20");

    //args.emplace_back("-v");
    //println("args {}", args);

    auto const res{ invoke_clang(args) };
    if(res.is_err())
    {
      println(stderr, "failed!");
      return err(res.expect_err());
    }

    println(stderr, "done!");
    return ok(output_path.c_str());
  }

  jtl::immutable_string default_target_triple()
  {
    static jtl::immutable_string result;
    if(!result.empty())
    {
      return result;
    }

    jank_debug_assert(runtime::__rt_ctx);
    return result
      = runtime::__rt_ctx->jit_prc.interpreter->getExecutionEngine()->getTargetTriple().str();
  }
}
