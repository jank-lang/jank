#include <iostream>
#include <jank/read/lex.hpp>
#include <jank/read/parse.hpp>
#include <jank/analyze/processor.hpp>
#include <jank/evaluate/context.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmacro-redefined"
#include <doctest/doctest.h>
#pragma clang diagnostic pop

/* TODO: Presence tests once defs can be evaluated. */
namespace jank::analyze::expr
{
  TEST_CASE("Unqualified")
  {
    runtime::context rt_ctx;
    processor anal_prc{ rt_ctx };
    evaluate::context eval_ctx{ rt_ctx };

    SUBCASE("Missing")
    {
      read::lex::processor l_prc{ "foo" };
      read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };

      CHECK_THROWS(anal_prc.analyze(p_prc.begin()->expect_ok()));
    }

    SUBCASE("Present")
    {
      {
        read::lex::processor l_prc{ "(def foo 1)" };
        read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
        eval_ctx.eval(anal_prc.analyze(p_prc.begin()->expect_ok()));
      }

      read::lex::processor l_prc{ "foo" };
      read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };

      auto const expr(anal_prc.analyze(p_prc.begin()->expect_ok()));
      auto const *var_deref_expr(boost::get<var_deref<expression>>(&expr.data));
      CHECK(var_deref_expr != nullptr);
      CHECK(var_deref_expr->var != nullptr);
      CHECK(var_deref_expr->var->name->name == "foo");
    }
  }

  TEST_CASE("Qualified")
  {
    runtime::context rt_ctx;
    processor anal_prc{ rt_ctx };
    evaluate::context eval_ctx{ rt_ctx };

    SUBCASE("Missing")
    {
      read::lex::processor l_prc{ "clojure.core/foo" };
      read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };

      CHECK_THROWS(anal_prc.analyze(p_prc.begin()->expect_ok()));
    }

    SUBCASE("Present")
    {
      {
        read::lex::processor l_prc{ "(def foo 1)" };
        read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
        eval_ctx.eval(anal_prc.analyze(p_prc.begin()->expect_ok()));
      }

      read::lex::processor l_prc{ "clojure.core/foo" };
      read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };

      auto const expr(anal_prc.analyze(p_prc.begin()->expect_ok()));
      auto const *var_deref_expr(boost::get<var_deref<expression>>(&expr.data));
      CHECK(var_deref_expr != nullptr);
      CHECK(var_deref_expr->var != nullptr);
      CHECK(var_deref_expr->var->name->name == "foo");
    }
  }
}
