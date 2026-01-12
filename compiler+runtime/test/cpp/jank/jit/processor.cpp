#include <filesystem>

#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <Interpreter/Compatibility.h>

#include <jtl/format/style.hpp>

#include <jank/util/scope_exit.hpp>
#include <jank/util/fmt/print.hpp>
#include <jank/read/lex.hpp>
#include <jank/read/parse.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>
#include <jank/runtime/obj/persistent_string.hpp>
#include <jank/runtime/obj/keyword.hpp>
#include <jank/runtime/obj/nil.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/runtime/core/equal.hpp>
#include <jank/analyze/processor.hpp>
#include <jank/jit/processor.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#include <doctest/doctest.h>

namespace jank::jit
{
  using runtime::__rt_ctx;

  struct failure
  {
    std::filesystem::path path;
    jtl::immutable_string error;
  };

  TEST_SUITE("jit")
  {
    TEST_CASE("files")
    {
      auto const cardinal_result(__rt_ctx->intern_keyword("success").expect_ok());
      usize test_count{};

      /* The functionality I want here is too complex for doctest to handle. Output should be
       * swallowed for expected scenarios, including expected failures, but the output should
       * be shown whenever something unexpected happens, so it can be debugged. On top of that,
       * individual failures being reported would be helpful. Thus all the manual tracking in
       * here. The outcome is nice, though. */
      native_vector<failure> failures;
      native_vector<std::filesystem::path> skips;

      /* We will intentionally introduce some bad C++ code and we don't want Clang outputting
       * compiler errors to stderr. If there are actual test issues which cause diagnostic
       * issues, the test will fail anyway and we can run it separately to see the errors. */
      auto &diag{ runtime::__rt_ctx->jit_prc.interpreter->getCompilerInstance()->getDiagnostics() };
      auto old_client{ diag.takeClient() };
      diag.setClient(new clang::IgnoringDiagConsumer{}, true);
      util::scope_exit const finally{ [&] { diag.setClient(old_client.release(), true); } };

      for(auto const &dir_entry : std::filesystem::recursive_directory_iterator("test/jank"))
      {
        if(!std::filesystem::is_regular_file(dir_entry.path()))
        {
          continue;
        }

        auto const filename(dir_entry.path().filename().string());

        if(filename.starts_with("."))
        {
          continue;
        }

        auto const expect_success(filename.starts_with("pass-"));
        auto const expect_failure(filename.starts_with("fail-"));
        auto const expect_throw(filename.starts_with("throw-"));
        auto const allow_failure(filename.starts_with("warn-"));
        auto const skip(filename.starts_with("skip-"));
        CHECK_MESSAGE((expect_success || expect_failure || allow_failure || expect_throw || skip),
                      "Test file needs to begin with pass- or fail- or throw- or warn- or skip-: ",
                      filename);
        ++test_count;

        /* TODO: Clear our rt_ctx for each run. Using the copy ctor leads to odd failures with
         * macros, likely due to interned keywords not being identical. */
        bool passed{ true };
        std::stringstream const captured_output;

        util::print("testing file {} => ", dir_entry.path().string());
        std::fflush(stdout);

        if(skip)
        {
          util::println("{}skipped{}", jtl::terminal_style::yellow, jtl::terminal_style::reset);
          skips.push_back(dir_entry.path());
          continue;
        }

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

          auto const result(
            __rt_ctx->eval_file(dir_entry.path().string()).unwrap_or(runtime::jank_nil()));
          if(!expect_success)
          {
            failures.push_back({ dir_entry.path(),
                                 util::format("Test failure was expected, but it passed with {}",
                                              runtime::to_code_string(result)) });
            passed = false;
          }
          else
          {
            if(!runtime::equal(result, cardinal_result))
            {
              failures.push_back(
                { dir_entry.path(),
                  util::format("Result is not :success: {}", runtime::to_string(result)) });
              passed = false;
            }
          }
        }
        /* TODO: Use JANK_TRY here? */
        catch(std::exception const &e)
        {
          if(expect_success || expect_throw)
          {
            failures.push_back(
              { dir_entry.path(), util::format("Exception thrown: {}", e.what()) });
            passed = false;
          }
        }
        catch(runtime::object_ref const e)
        {
          if(expect_success || (expect_throw && !runtime::equal(e, cardinal_result)))
          {
            failures.push_back(
              { dir_entry.path(), util::format("Exception thrown: {}", runtime::to_string(e)) });
            passed = false;
          }
          else if(expect_failure && runtime::equal(e, cardinal_result))
          {
            failures.push_back(
              { dir_entry.path(),
                util::format("Expected failure, thrown: {}", runtime::to_string(e)) });
            passed = false;
          }
        }
        catch(runtime::obj::keyword_ref const e)
        {
          if(!expect_throw || !runtime::equal(e.erase(), cardinal_result))
          {
            failures.push_back(
              { dir_entry.path(), util::format("Exception thrown: {}", runtime::to_string(e)) });
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
          util::println("{}allowed failure{}",
                        jtl::terminal_style::yellow,
                        jtl::terminal_style::reset);
        }
        else if(passed)
        {
          util::println("{}success{}", jtl::terminal_style::green, jtl::terminal_style::reset);
        }
        else
        {
          util::println("{}failure{}", jtl::terminal_style::red, jtl::terminal_style::reset);
          std::cerr << captured_output.rdbuf() << "\n";
          std::cerr.flush();
        }
      }

      util::println(
        "\n===============================================================================");
      CHECK(failures.empty());
      for(auto const &f : skips)
      {
        util::print("{}skip{}: {}\n",
                    jtl::terminal_style::yellow,
                    jtl::terminal_style::reset,
                    f.string());
      }
      for(auto const &f : failures)
      {
        util::print("{}failure{}: {}\n\t{}\n",
                    jtl::terminal_style::red,
                    jtl::terminal_style::reset,
                    f.path.string(),
                    f.error);
      }
      util::print("tested {} jank files with {}{} skips{} and {}{} failures{}\n",
                  test_count,
                  (skips.empty() ? jtl::terminal_style::reset : jtl::terminal_style::yellow),
                  skips.size(),
                  jtl::terminal_style::reset,
                  (failures.empty() ? jtl::terminal_style::reset : jtl::terminal_style::red),
                  failures.size(),
                  jtl::terminal_style::reset);
    }
  }
}
