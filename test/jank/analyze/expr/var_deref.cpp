#include <doctest/doctest.h>

#include <iostream>
#include <jank/read/lex.hpp>
#include <jank/read/parse.hpp>
#include <jank/analyze/processor.hpp>

/* TODO: Presence tests once defs can be evaluated. */
namespace jank::analyze::expr
{
  TEST_CASE("Unqualified")
  {
    runtime::context rt_ctx;
    processor anal_prc{ rt_ctx };

    SUBCASE("Missing")
    {
      read::lex::processor l_prc{ "foo" };
      read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };

      CHECK_THROWS(anal_prc.analyze(p_prc.begin()->expect_ok()));
    }

    //SUBCASE("Present")
    //{
    //  read::lex::processor l_prc{ "foo" };
    //  read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };

    //  auto const expr(anal_prc.analyze(p_prc.begin()->expect_ok()));
    //  auto const *var_deref_expr(std::get_if<var_deref<expression>>(&expr.data));
    //  CHECK(var_deref_expr != nullptr);
    //  CHECK(var_deref_expr->var != nullptr);
    //}
  }

  TEST_CASE("Qualified")
  {
    runtime::context rt_ctx;
    processor anal_prc{ rt_ctx };

    SUBCASE("Missing")
    {
      read::lex::processor l_prc{ "foo/bar" };
      read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };

      CHECK_THROWS(anal_prc.analyze(p_prc.begin()->expect_ok()));
    }

    //SUBCASE("Present")
    //{
    //  read::lex::processor l_prc{ "foo/bar" };
    //  read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };

    //  auto const expr(anal_prc.analyze(p_prc.begin()->expect_ok()));
    //  auto const *var_deref_expr(std::get_if<var_deref<expression>>(&expr.data));
    //  CHECK(var_deref_expr != nullptr);
    //  CHECK(var_deref_expr->var != nullptr);
    //}
  }
}
