#include <jank/read/lex.hpp>
#include <jank/read/parse.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/analyze/processor.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#include <doctest/doctest.h>

/* TODO: Test cases
 * Redefine var
 * Reference unknown sym in value
 */
namespace jank::analyze::expr
{
  TEST_CASE("Declare")
  {
    read::lex::processor l_prc{ "(def *meow-meow*)" };
    read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
    runtime::context rt_ctx;
    context an_ctx{ rt_ctx };
    processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

    auto const fn_expr(an_prc.result(an_ctx).expect_ok().unwrap());
    auto const expr(boost::get<function<expression>>(fn_expr.data).body.body.front());
    auto const *def_expr(boost::get<def<expression>>(&expr.data));
    CHECK(def_expr != nullptr);
    CHECK(def_expr->name != nullptr);
    CHECK(def_expr->name->equal(runtime::obj::symbol{ "clojure.core", "*meow-meow*" }));
    CHECK(def_expr->value.is_none());
  }

  TEST_CASE("Basic")
  {
    read::lex::processor l_prc{ "(def foo 777)" };
    read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
    runtime::context rt_ctx;
    context an_ctx{ rt_ctx };
    processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

    auto const fn_expr(an_prc.result(an_ctx).expect_ok().unwrap());
    auto const expr(boost::get<function<expression>>(fn_expr.data).body.body.front());
    auto const *def_expr(boost::get<def<expression>>(&expr.data));
    CHECK(def_expr != nullptr);
    CHECK(def_expr->name != nullptr);
    CHECK(def_expr->name->equal(runtime::obj::symbol{ "clojure.core", "foo" }));
    CHECK(boost::get<expr::primitive_literal<expression>>(&def_expr->value.unwrap()->data) != nullptr);
    CHECK(boost::get<expr::primitive_literal<expression>>(def_expr->value.unwrap()->data).data->equal(runtime::obj::integer{ 777 }));

    SUBCASE("Lifting")
    {
      CHECK_EQ(def_expr->frame->lifted_vars.size(), 1);
      CHECK_NE(def_expr->frame->lifted_vars.find(runtime::obj::symbol::create("clojure.core", "foo")), def_expr->frame->lifted_vars.end());
      CHECK_EQ(def_expr->frame->lifted_constants.size(), 1);
      CHECK_NE(def_expr->frame->lifted_constants.find(runtime::obj::integer::create(777)), def_expr->frame->lifted_constants.end());
    }
  }

  TEST_CASE("Qualified symbol")
  {
    read::lex::processor l_prc{ "(def bar/foo 777)" };
    read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
    runtime::context rt_ctx;
    context an_ctx{ rt_ctx };
    processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

    CHECK(an_prc.result(an_ctx).is_err());
  }

  TEST_CASE("Arities")
  {
    runtime::context rt_ctx;
    context an_ctx{ rt_ctx };

    SUBCASE("Extra value")
    {
      read::lex::processor l_prc{ "(def foo 1 2)" };
      read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
      processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

      CHECK(an_prc.result(an_ctx).is_err());
    }
  }
}
