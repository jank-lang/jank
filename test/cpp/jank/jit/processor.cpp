#include <filesystem>

#include <boost/algorithm/string/predicate.hpp>

#include <jank/util/mapped_file.hpp>
#include <jank/read/lex.hpp>
#include <jank/read/parse.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/obj/vector.hpp>
#include <jank/runtime/obj/string.hpp>
#include <jank/runtime/obj/keyword.hpp>
#include <jank/analyze/processor.hpp>
#include <jank/jit/processor.hpp>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <cling/Interpreter/Interpreter.h>
#include <cling/Interpreter/Value.h>
#pragma clang diagnostic pop

/* This must go last; doctest and glog both define CHECK and family. */
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <doctest/doctest.h>
#pragma clang diagnostic pop

namespace jank::jit
{
  TEST_CASE("Files")
  {
    auto const cardinal_result
    (runtime::obj::keyword::create(runtime::obj::symbol{ "", "success" }, true));

    for(auto const &dir_entry : std::filesystem::recursive_directory_iterator("test/jank"))
    {
      if(!dir_entry.is_regular_file())
      { continue; }

      auto const expect_success
      (boost::algorithm::starts_with(dir_entry.path().filename().string(), "pass-"));
      auto const expect_failure
      (boost::algorithm::starts_with(dir_entry.path().filename().string(), "fail-"));
      CHECK_MESSAGE
      (
        (expect_success || expect_failure),
        "Test file needs to begin with pass- or fail-: ",
        dir_entry
      );

      //std::cout << "testing " << dir_entry << std::endl;

      jank::runtime::context rt_ctx;
      jank::analyze::context an_ctx{ rt_ctx };

      rt_ctx.eval_prelude(an_ctx);

      auto const mfile(jank::util::map_file(dir_entry.path().string()));
      jank::read::lex::processor l_prc{ { mfile.expect_ok().head, mfile.expect_ok().size } };
      jank::read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
      jank::analyze::processor an_prc
      { rt_ctx, p_prc.begin(), p_prc.end() };
      jank::codegen::processor cg_prc
      {
        rt_ctx,
        an_ctx,
        an_prc.result(an_ctx).expect_ok_move().unwrap()
      };
      //std::cout << cg_prc.declaration_str() << std::endl;
      //std::cout << cg_prc.expression_str() << std::endl;

      try
      {
        jank::jit::processor jit_prc;
        auto const result(jit_prc.eval(rt_ctx, cg_prc));
        CHECK_MESSAGE(expect_success, "Test passed when a failure was expected: ", dir_entry);
        CHECK(result.is_ok());
        CHECK(result.expect_ok().is_some());
        CHECK(result.expect_ok().unwrap()->equal(cardinal_result));
      }
      catch(...)
      { CHECK(!expect_success); }
    }
  }
}
