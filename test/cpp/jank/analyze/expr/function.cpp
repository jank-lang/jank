#include <doctest/doctest.h>

#include <jank/read/lex.hpp>
#include <jank/read/parse.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/analyze/processor.hpp>

namespace jank::analyze::expr
{
  TEST_CASE("Empty")
  {
    read::lex::processor l_prc{ "(fn* [])" };
    read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
    runtime::context rt_ctx;
    context an_ctx{ rt_ctx };
    processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

    auto const fn_expr(an_prc.result(an_ctx).expect_ok().unwrap());
    auto const expr(boost::get<function<expression>>(fn_expr.data).body.body.front());
    auto const *typed_expr(boost::get<function<expression>>(&expr.data));
    CHECK(typed_expr != nullptr);
    CHECK(typed_expr->frame->locals.empty());
    CHECK(typed_expr->body.body.empty());
  }

  TEST_CASE("Parameters")
  {
    runtime::context rt_ctx;
    context an_ctx{ rt_ctx };

    SUBCASE("With local references")
    {
      read::lex::processor l_prc{ "(fn* [a b] a b)" };
      read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
      processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

      auto const fn_expr(an_prc.result(an_ctx).expect_ok().unwrap());
      auto const expr(boost::get<function<expression>>(fn_expr.data).body.body.front());
      auto const *typed_expr(boost::get<function<expression>>(&expr.data));
      CHECK(typed_expr != nullptr);
      CHECK(typed_expr->frame->locals.size() == 2);
      CHECK(typed_expr->body.body.size() == 2);
    }

    SUBCASE("Missing - nothing else")
    {
      read::lex::processor l_prc{ "(fn*)" };
      read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
      processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

      CHECK(an_prc.result(an_ctx).is_err());
    }

    SUBCASE("Missing - with body")
    {
      read::lex::processor l_prc{ "(fn* 1)" };
      read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
      processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

      CHECK(an_prc.result(an_ctx).is_err());
    }

    SUBCASE("Not all symbols")
    {
      read::lex::processor l_prc{ "(fn* [a 1])" };
      read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
      processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

      CHECK(an_prc.result(an_ctx).is_err());
    }

    SUBCASE("Qualified symbol")
    {
      read::lex::processor l_prc{ "(fn* [foo.bar/a])" };
      read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
      processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

      CHECK(an_prc.result(an_ctx).is_err());
    }

    SUBCASE("Too many")
    {
      read::lex::processor l_prc{ "(fn* [a b c d e f g h i j k])" };
      read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
      processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

      CHECK(an_prc.result(an_ctx).is_err());
    }
  }
}
