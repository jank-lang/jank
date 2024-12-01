#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>

#include <fmt/color.h>

#include <jank/util/mapped_file.hpp>
#include <jank/util/scope_exit.hpp>
#include <jank/read/lex.hpp>
#include <jank/read/parse.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>
#include <jank/runtime/obj/persistent_string.hpp>
#include <jank/runtime/obj/keyword.hpp>
#include <jank/analyze/processor.hpp>
#include <jank/jit/processor.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#include <doctest/doctest.h>

namespace jank::jit
{
  using runtime::__rt_ctx;

  struct failure
  {
    boost::filesystem::path path;
    native_persistent_string error;
  };

  TEST_SUITE("jit")
  {
    TEST_CASE("files")
    {
      auto const cardinal_result(__rt_ctx->intern_keyword("success").expect_ok());
      size_t test_count{};

      /* The functionality I want here is too complex for doctest to handle. Output should be
       * swallowed for expected scenarios, including expected failures, but the output should
       * be shown whenever something unexpected happens, so it can be debugged. On top of that,
       * individual failures being reported would be helpful. Thus all the manual tracking in
       * here. The outcome is nice, though. */
      native_vector<failure> failures;

      /* TODO: Disable diagnostic printing. The below works in non-test code, but crashes
       * in test code. Even when it did work, it had two issues.
       *
       * 1. It didn't capture the output to display later
       * 2. It still showed the "error: Parsing failed." line
       *
       */
      //std::vector<char const *> args{ "clang++", "-w" };
      //static llvm::IntrusiveRefCntPtr<clang::DiagnosticOptions> diag_opts{
      //  clang::CreateAndPopulateDiagOpts(args)
      //};
      //static clang::TextDiagnosticPrinter diag_printer(llvm::nulls(), &*diag_opts);
      //static clang::DiagnosticsEngine diags(
      //  llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs>(new clang::DiagnosticIDs()),
      //  &*diag_opts,
      //  &diag_printer,
      //  false);
      /* TODO: Why does this crash? Should be what we want. */
      //diags.setSuppressAllDiagnostics(true);
      //__rt_ctx->jit_prc.interpreter->getCompilerInstance()->setDiagnostics(&diags);

      for(auto const &dir_entry : boost::filesystem::recursive_directory_iterator("test/jank"))
      {
        if(!boost::filesystem::is_regular_file(dir_entry.path()))
        {
          continue;
        }

        auto const filename(dir_entry.path().filename().string());
        auto const expect_success(boost::algorithm::starts_with(filename, "pass-"));
        auto const expect_failure(boost::algorithm::starts_with(filename, "fail-"));
        auto const expect_throw(boost::algorithm::starts_with(filename, "throw-"));
        auto const allow_failure(boost::algorithm::starts_with(filename, "warn-"));
        CHECK_MESSAGE((expect_success || expect_failure || allow_failure || expect_throw),
                      "Test file needs to begin with pass- or fail- or throw- or warn-: ",
                      filename);
        ++test_count;

        /* TODO: Clear our rt_ctx for each run. Using the copy ctor leads to odd failures with
         * macros, likely due to interned keywords not being identical. */
        bool passed{ true };
        std::stringstream captured_output;

        fmt::print("testing file {} => ", dir_entry.path().string());

        try
        {
          /* Silence ouptut when running these. This include compilation errors from Clang,
           * since we're going to intentionally make that happen. */
          std::streambuf * const old_cout{ std::cout.rdbuf(captured_output.rdbuf()) };
          std::streambuf * const old_cerr{ std::cerr.rdbuf(captured_output.rdbuf()) };
          util::scope_exit const _{ [=]() {
            std::cout.rdbuf(old_cout);
            std::cerr.rdbuf(old_cerr);
          } };

          auto const result(__rt_ctx->eval_file(dir_entry.path().string()));
          if(!expect_success)
          {
            failures.push_back(
              { dir_entry.path(),
                fmt::format("Test failure was expected, but it passed with {}",
                            (result == nullptr ? "nullptr" : runtime::to_string(result))) });
            passed = false;
          }
          else
          {
            if(result == nullptr)
            {
              failures.push_back({ dir_entry.path(), "Returned object is nullptr" });
              passed = false;
            }
            else if(!runtime::equal(result, cardinal_result))
            {
              failures.push_back(
                { dir_entry.path(),
                  fmt::format("Result is not :success: {}", runtime::to_string(result)) });
              passed = false;
            }
          }
        }
        catch(std::exception const &e)
        {
          if(expect_success || expect_throw)
          {
            failures.push_back({ dir_entry.path(), fmt::format("Exception thrown: {}", e.what()) });
            passed = false;
          }
        }
        catch(runtime::object_ptr const e)
        {
          if(expect_success || (expect_throw && !runtime::equal(e, cardinal_result)))
          {
            failures.push_back(
              { dir_entry.path(), fmt::format("Exception thrown: {}", runtime::to_string(e)) });
            passed = false;
          }
          else if(expect_failure && runtime::equal(e, cardinal_result))
          {
            failures.push_back(
              { dir_entry.path(),
                fmt::format("Expected failure, thrown: {}", runtime::to_string(e)) });
            passed = false;
          }
        }
        catch(runtime::obj::keyword_ptr const e)
        {
          if(!expect_throw || !runtime::equal(e, cardinal_result))
          {
            failures.push_back(
              { dir_entry.path(), fmt::format("Exception thrown: {}", runtime::to_string(e)) });
            passed = false;
          }
        }
        catch(...)
        {
          if(expect_success || expect_throw)
          {
            failures.push_back({ dir_entry.path(), "Unknown exception thrown" });
            passed = false;
          }
        }

        if(allow_failure)
        {
          fmt::print(fmt::fg(fmt::color::orange), "allowed failure\n");
        }
        else if(passed)
        {
          fmt::print(fmt::fg(fmt::color::green), "success\n");
        }
        else
        {
          fmt::print(fmt::fg(fmt::color::red), "failure\n");
          std::cerr << captured_output.rdbuf() << "\n";
          std::cerr.flush();
        }
      }
      for(auto const f : failures)
      {
        std::cout << f.path << std::endl;
        std::cout << f.error << std::endl;
      }

      CHECK(failures.empty());
      for(auto const &f : failures)
      {
        fmt::print("{}: {} {}\n",
                   fmt::styled("failure", fmt::fg(fmt::color::red)),
                   f.path.string(),
                   f.error);
      }
      fmt::print("tested {} jank files\n", test_count);
    }
  }
}
