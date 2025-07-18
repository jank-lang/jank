#include <filesystem>
#include <fstream>

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
#include <jank/util/fmt/print.hpp>

namespace jank::util
{
  /* Whenever we need to invoke Clang, at run time, we need to make sure we're using
   * a matching Clang to what was used to build jank. We give this three attempts.
   *
   * 1. jank is configured with a JANK_CLANG_PATH which says the path to the exact
   *    Clang which was used to build jank. In most cases of installing jank on
   *    someone's machine, this should work well.
   *
   * 2. jank is configured with a JANK_CLANG_MAJOR_VERSION which says the major
   *    Clang version which was used to build jank. At this point, we don't care
   *    about minor version differences. So, if we were built with Clang 20, we
   *    will look for clang++-20 on the user's PATH.
   *
   * 3. Finally, if neither of the above is found, we'll look for clang++ on the
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

    auto const versioned_path{ llvm::sys::findProgramByName("clang++-" JANK_CLANG_MAJOR_VERSION) };
    if(versioned_path)
    {
      return result = *versioned_path;
    }

    auto const unversioned_path{ llvm::sys::findProgramByName("clang++") };
    if(unversioned_path)
    {
      auto const tmp{ std::filesystem::temp_directory_path() };
      std::string path_tmp{ tmp / "jank-clang-XXXXXX" };
      mkstemp(path_tmp.data());
      auto const proc_code{ llvm::sys::ExecuteAndWait(*unversioned_path,
                                                      { (*unversioned_path).c_str(), "--version" },
                                                      std::nullopt,
                                                      { std::nullopt, path_tmp, std::nullopt }) };
      if(proc_code < 0)
      {
        return none;
      }

      std::string version_line;
      std::ifstream ifs{ path_tmp };
      std::getline(ifs, version_line);
      auto const found{ version_line.find("version " JANK_CLANG_MAJOR_VERSION) };
      if(found != std::string::npos)
      {
        return result = unversioned_path->c_str();
      }
    }

    return none;
  }

  jtl::result<void, jtl::immutable_string> invoke_clang(std::vector<char const *> const &args)
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
      return err("Unable to find a suitable Clang " JANK_CLANG_MAJOR_VERSION " binary.");
    }
    auto const &clang_path{ clang_path_res.unwrap() };

    /* Building the driver doesn't actually run the commands yet. All of the flags will
     * be checked, though. */
    clang::driver::Driver driver{ clang_path.c_str(), target_triple, diags, "jank", vfs };
    driver.setCheckInputsExist(true);

    auto const compilation_result{ driver.BuildCompilation(args) };
    if(!compilation_result || compilation_result->containsError())
    {
      return err(format("Failed to build Clang steps.\n{}", buffer));
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
      return err(format("Clang failed with errors.\n{}", buffer));
    }

    return ok();
  }
}
