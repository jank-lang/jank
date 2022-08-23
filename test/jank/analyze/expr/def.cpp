#include <jank/read/lex.hpp>
#include <jank/read/parse.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/analyze/processor.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmacro-redefined"
#include <doctest/doctest.h>
#pragma clang diagnostic pop

/* TODO: Test cases
 * Redefine var
 * Reference unknown sym in value
 */
namespace jank::analyze::expr
{
  TEST_CASE("Basic")
  {
    read::lex::processor l_prc{ "(def foo 777)" };
    read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
    runtime::context rt_ctx;
    context an_ctx{ rt_ctx };
    processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

    auto const expr(an_prc.next(an_ctx).expect_ok().unwrap());
    auto const *def_expr(boost::get<def<expression>>(&expr.data));
    CHECK(def_expr != nullptr);
    CHECK(def_expr->name != nullptr);
    CHECK(def_expr->name->equal(runtime::obj::symbol{ "clojure.core/foo" }));
    CHECK(boost::get<expr::primitive_literal<expression>>(&def_expr->value->data) != nullptr);
    CHECK(boost::get<expr::primitive_literal<expression>>(def_expr->value->data).data->equal(runtime::obj::integer{ 777 }));

    SUBCASE("Lifting")
    {
      CHECK_EQ(an_ctx.tracked_refs.lifted_vars.size(), 1);
      CHECK_NE(an_ctx.tracked_refs.lifted_vars.find(runtime::obj::symbol::create("clojure.core/foo")), an_ctx.tracked_refs.lifted_vars.end());
      CHECK_EQ(an_ctx.tracked_refs.lifted_constants.size(), 1);
      CHECK_NE(an_ctx.tracked_refs.lifted_constants.find(runtime::obj::integer::create(777)), an_ctx.tracked_refs.lifted_constants.end());
    }
  }

  TEST_CASE("Qualified symbol")
  {
    read::lex::processor l_prc{ "(def bar/foo 777)" };
    read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
    runtime::context rt_ctx;
    context an_ctx{ rt_ctx };
    processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

    CHECK(an_prc.next(an_ctx).is_err());
  }

  TEST_CASE("Arities")
  {
    runtime::context rt_ctx;
    context an_ctx{ rt_ctx };

    SUBCASE("Missing value")
    {
      read::lex::processor l_prc{ "(def foo)" };
      read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
      processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

      CHECK(an_prc.next(an_ctx).is_err());
    }

    SUBCASE("Extra value")
    {
      read::lex::processor l_prc{ "(def foo 1 2)" };
      read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
      processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

      CHECK(an_prc.next(an_ctx).is_err());
    }
  }
}
