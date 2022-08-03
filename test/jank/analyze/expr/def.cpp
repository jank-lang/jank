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
    context anal_ctx{ rt_ctx };
    processor anal_prc{ rt_ctx };

    auto const expr(anal_prc.analyze(p_prc.begin()->expect_ok(), anal_ctx));
    auto const *def_expr(boost::get<def<expression>>(&expr.data));
    CHECK(def_expr != nullptr);
    CHECK(def_expr->name != nullptr);
    CHECK(def_expr->name->equal(runtime::obj::symbol{ "foo" }));
    CHECK(boost::get<expr::primitive_literal<expression>>(&def_expr->value.data) != nullptr);
    CHECK(boost::get<expr::primitive_literal<expression>>(def_expr->value.data).data->equal(runtime::obj::integer{ 777 }));

    SUBCASE("Lifting")
    {
      CHECK_EQ(anal_ctx.lifted_vars.size(), 1);
      CHECK_NE(anal_ctx.lifted_vars.find(runtime::obj::symbol::create("clojure.core/foo")), anal_ctx.lifted_vars.end());
      CHECK_EQ(anal_ctx.lifted_constants.size(), 1);
      CHECK_NE(anal_ctx.lifted_constants.find(runtime::obj::integer::create(777)), anal_ctx.lifted_constants.end());
    }
  }

  TEST_CASE("Qualified symbol")
  {
    read::lex::processor l_prc{ "(def bar/foo 777)" };
    read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
    runtime::context rt_ctx;
    context anal_ctx{ rt_ctx };
    processor anal_prc{ rt_ctx };

    CHECK_THROWS(anal_prc.analyze(p_prc.begin()->expect_ok(), anal_ctx));
  }

  TEST_CASE("Arities")
  {
    runtime::context rt_ctx;
    context anal_ctx{ rt_ctx };
    processor anal_prc{ rt_ctx };

    SUBCASE("Missing value")
    {
      read::lex::processor l_prc{ "(def foo)" };
      read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };

      CHECK_THROWS(anal_prc.analyze(p_prc.begin()->expect_ok(), anal_ctx));
    }

    SUBCASE("Extra value")
    {
      read::lex::processor l_prc{ "(def foo 1 2)" };
      read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };

      CHECK_THROWS(anal_prc.analyze(p_prc.begin()->expect_ok(), anal_ctx));
    }
  }
}
