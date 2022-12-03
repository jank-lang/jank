#include <doctest/doctest.h>

#include <jank/read/lex.hpp>
#include <jank/read/parse.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/analyze/processor.hpp>

namespace jank::analyze::expr
{
  TEST_CASE("Basic")
  {
    read::lex::processor l_prc{ "(if true 1 2)" };
    read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
    runtime::context rt_ctx;
    context an_ctx{ rt_ctx };
    processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

    auto const fn_expr(an_prc.result(an_ctx).expect_ok());
    auto const expr(boost::get<function<expression>>(fn_expr->data).arities[0].body.body.front());
    auto const *typed_expr(boost::get<if_<expression>>(&expr->data));
    CHECK(typed_expr != nullptr);
    CHECK(typed_expr->condition != nullptr);
    CHECK(typed_expr->then != nullptr);
    CHECK(typed_expr->else_.unwrap() != nullptr);
  }

  TEST_CASE("No else")
  {
    read::lex::processor l_prc{ "(if true 1)" };
    read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
    runtime::context rt_ctx;
    context an_ctx{ rt_ctx };
    processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

    auto const fn_expr(an_prc.result(an_ctx).expect_ok());
    auto const expr(boost::get<function<expression>>(fn_expr->data).arities[0].body.body.front());
    auto const *typed_expr(boost::get<if_<expression>>(&expr->data));
    CHECK(typed_expr != nullptr);
    CHECK(typed_expr->condition != nullptr);
    CHECK(typed_expr->then != nullptr);
    CHECK(typed_expr->else_.is_none());
  }

  TEST_CASE("Non-bool condition")
  {
    read::lex::processor l_prc{ "(if [:ok] 1)" };
    read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
    runtime::context rt_ctx;
    context an_ctx{ rt_ctx };
    processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

    auto const fn_expr(an_prc.result(an_ctx).expect_ok());
    auto const expr(boost::get<function<expression>>(fn_expr->data).arities[0].body.body.front());
    auto const *typed_expr(boost::get<if_<expression>>(&expr->data));
    CHECK(typed_expr != nullptr);
    CHECK(typed_expr->condition != nullptr);
    CHECK(typed_expr->then != nullptr);
    CHECK(typed_expr->else_.is_none());
  }

  TEST_CASE("No then")
  {
    read::lex::processor l_prc{ "(if true)" };
    read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
    runtime::context rt_ctx;
    context an_ctx{ rt_ctx };
    processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

    CHECK(an_prc.result(an_ctx).is_err());
  }

  TEST_CASE("No condition")
  {
    read::lex::processor l_prc{ "(if)" };
    read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
    runtime::context rt_ctx;
    context an_ctx{ rt_ctx };
    processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

    CHECK(an_prc.result(an_ctx).is_err());
  }

  TEST_CASE("Extra inputs")
  {
    read::lex::processor l_prc{ "(if 1 2 3 4)" };
    read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
    runtime::context rt_ctx;
    context an_ctx{ rt_ctx };
    processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

    CHECK(an_prc.result(an_ctx).is_err());
  }
}
