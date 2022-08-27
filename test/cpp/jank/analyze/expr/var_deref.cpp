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

namespace jank::analyze::expr
{
  TEST_CASE("Unqualified")
  {
    runtime::context rt_ctx;
    context an_ctx{ rt_ctx };

    SUBCASE("Missing")
    {
      read::lex::processor l_prc{ "foo" };
      read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
      processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

      CHECK(an_prc.next(an_ctx).is_err());
    }

    SUBCASE("Present")
    {
      rt_ctx.eval_string("(def foo 1)", an_ctx);

      read::lex::processor l_prc{ "foo" };
      read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
      processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

      auto const expr(an_prc.next(an_ctx).expect_ok().unwrap());
      auto const *var_deref_expr(boost::get<var_deref<expression>>(&expr.data));
      CHECK(var_deref_expr != nullptr);
      CHECK(var_deref_expr->qualified_name != nullptr);
      CHECK(*var_deref_expr->qualified_name == runtime::obj::symbol{ "clojure.core", "foo" });
    }
  }

  TEST_CASE("Qualified")
  {
    runtime::context rt_ctx;
    context an_ctx{ rt_ctx };

    SUBCASE("Missing")
    {
      read::lex::processor l_prc{ "clojure.core/foo" };
      read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
      processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

      CHECK(an_prc.next(an_ctx).is_err());
    }

    SUBCASE("Present")
    {
      rt_ctx.eval_string("(def foo 1)", an_ctx);

      read::lex::processor l_prc{ "clojure.core/foo" };
      read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
      processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

      auto const expr(an_prc.next(an_ctx).expect_ok().unwrap());
      auto const *var_deref_expr(boost::get<var_deref<expression>>(&expr.data));
      CHECK(var_deref_expr != nullptr);
      CHECK(var_deref_expr->qualified_name != nullptr);
      CHECK(*var_deref_expr->qualified_name == runtime::obj::symbol{ "clojure.core", "foo" });
    }
  }
}
