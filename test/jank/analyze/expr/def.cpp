#include <doctest/doctest.h>

#include <jank/read/lex.hpp>
#include <jank/read/parse.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/analyze/processor.hpp>

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
    processor anal_prc{ rt_ctx };

    auto const expr(anal_prc.analyze(p_prc.begin()->expect_ok()));
    auto const *def_expr(boost::get<def<expression>>(&expr.data));
    CHECK(def_expr != nullptr);
    CHECK(def_expr->name != nullptr);
    CHECK(def_expr->name->equal(runtime::obj::symbol{ "foo" }));
    CHECK(def_expr->value != nullptr);
    CHECK(def_expr->value->equal(runtime::obj::integer{ 777 }));
  }

  TEST_CASE("Qualified symbol")
  {
    read::lex::processor l_prc{ "(def bar/foo 777)" };
    read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
    runtime::context rt_ctx;
    processor anal_prc{ rt_ctx };

    CHECK_THROWS(anal_prc.analyze(p_prc.begin()->expect_ok()));
  }

  TEST_CASE("Arities")
  {
    runtime::context rt_ctx;
    processor anal_prc{ rt_ctx };

    SUBCASE("Missing value")
    {
      read::lex::processor l_prc{ "(def foo)" };
      read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };

      CHECK_THROWS(anal_prc.analyze(p_prc.begin()->expect_ok()));
    }

    SUBCASE("Extra value")
    {
      read::lex::processor l_prc{ "(def foo 1 2)" };
      read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };

      CHECK_THROWS(anal_prc.analyze(p_prc.begin()->expect_ok()));
    }
  }
}
