#include <jank/read/lex.hpp>
#include <jank/read/parse.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/analyze/processor.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <doctest/doctest.h>
#pragma clang diagnostic pop

namespace jank::analyze::expr
{
  TEST_CASE("No arg")
  {
    read::lex::processor l_prc{ "(native/raw)" };
    read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
    runtime::context rt_ctx;
    context an_ctx{ rt_ctx };
    processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

    CHECK(an_prc.result(an_ctx).is_err());
  }

  TEST_CASE("Invalid arg type")
  {
    read::lex::processor l_prc{ "(native/raw wow-much-raw)" };
    read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
    runtime::context rt_ctx;
    context an_ctx{ rt_ctx };
    processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

    CHECK(an_prc.result(an_ctx).is_err());
  }

  TEST_CASE("Empty string")
  {
    read::lex::processor l_prc{ R"((native/raw ""))" };
    read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
    runtime::context rt_ctx;
    context an_ctx{ rt_ctx };
    processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

    auto const fn_expr(an_prc.result(an_ctx).expect_ok().unwrap());
    auto const expr(boost::get<function<expression>>(fn_expr.data).body.body.front());
    auto const *native_raw_expr(boost::get<native_raw<expression>>(&expr.data));
    CHECK(native_raw_expr != nullptr);
    CHECK(native_raw_expr->code != nullptr);
    CHECK(native_raw_expr->code->data.empty());
  }

  TEST_CASE("C++ code")
  {
    read::lex::processor l_prc{ R"((native/raw "std::cout << 77;"))" };
    read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
    runtime::context rt_ctx;
    context an_ctx{ rt_ctx };
    processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

    auto const fn_expr(an_prc.result(an_ctx).expect_ok().unwrap());
    auto const expr(boost::get<function<expression>>(fn_expr.data).body.body.front());
    auto const *native_raw_expr(boost::get<native_raw<expression>>(&expr.data));
    CHECK(native_raw_expr != nullptr);
    CHECK(native_raw_expr->code != nullptr);
    CHECK(native_raw_expr->code->data == "std::cout << 77;");
  }
}
