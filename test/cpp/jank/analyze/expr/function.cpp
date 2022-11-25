#include <doctest/doctest.h>

#include <jank/read/lex.hpp>
#include <jank/read/parse.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/analyze/processor.hpp>

/* TODO: Negative test cases:
 * 1. fn* used as a value */
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
    auto const expr(boost::get<function<expression>>(fn_expr.data).arities[0].body.body.front());
    auto const *typed_expr(boost::get<function<expression>>(&expr.data));
    CHECK(typed_expr != nullptr);
    CHECK(typed_expr->arities[0].frame->locals.empty());
    CHECK(typed_expr->arities[0].body.body.empty());
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
      auto const expr(boost::get<function<expression>>(fn_expr.data).arities[0].body.body.front());
      auto const *typed_expr(boost::get<function<expression>>(&expr.data));
      CHECK(typed_expr != nullptr);
      CHECK(typed_expr->arities[0].frame->locals.size() == 2);
      CHECK(typed_expr->arities[0].body.body.size() == 2);
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

  TEST_CASE("Missing arg vector")
  {
    read::lex::processor l_prc{ "(fn*)" };
    read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
    runtime::context rt_ctx;
    context an_ctx{ rt_ctx };
    processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

    CHECK(an_prc.result(an_ctx).is_err());
  }

  TEST_CASE("Invalid arg vector")
  {
    read::lex::processor l_prc{ "(fn* :args)" };
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

    SUBCASE("Single, with wrapper list")
    {
      read::lex::processor l_prc{ "(fn* ([] 1))" };
      read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
      processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

      auto const fn_expr(an_prc.result(an_ctx).expect_ok().unwrap());
      auto const expr(boost::get<function<expression>>(fn_expr.data).arities[0].body.body.front());
      auto const *typed_expr(boost::get<function<expression>>(&expr.data));
      CHECK(typed_expr != nullptr);
      CHECK(typed_expr->arities[0].frame->locals.size() == 0);
      CHECK(typed_expr->arities[0].body.body.size() == 1);
    }

    SUBCASE("Multiple")
    {
      read::lex::processor l_prc{ "(fn* ([] 1) ([a] a) ([a b] [a b]))" };
      read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
      processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

      auto const fn_expr(an_prc.result(an_ctx).expect_ok().unwrap());
      auto const expr(boost::get<function<expression>>(fn_expr.data).arities[0].body.body.front());
      auto const *typed_expr(boost::get<function<expression>>(&expr.data));
      CHECK(typed_expr != nullptr);
      CHECK(typed_expr->arities[0].frame->locals.size() == 0);
      CHECK(typed_expr->arities[0].body.body.size() == 1);
      CHECK(typed_expr->arities[1].frame->locals.size() == 1);
      CHECK(typed_expr->arities[1].body.body.size() == 1);
      CHECK(typed_expr->arities[2].frame->locals.size() == 2);
      CHECK(typed_expr->arities[2].body.body.size() == 1);
    }

    SUBCASE("Duplicate")
    {
      read::lex::processor l_prc{ "(fn* ([] 1) ([] 2))" };
      read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
      processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

      CHECK(an_prc.result(an_ctx).is_err());
    }

    SUBCASE("Invalid arity type")
    {
      read::lex::processor l_prc{ "(fn* ([] 1) [[a] a])" };
      read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
      processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

      CHECK(an_prc.result(an_ctx).is_err());
    }
  }

  TEST_CASE("Variadic")
  {
    runtime::context rt_ctx;
    context an_ctx{ rt_ctx };

    SUBCASE("Single")
    {
      read::lex::processor l_prc{ "(fn* ([& args] 1))" };
      read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
      processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

      auto const fn_expr(an_prc.result(an_ctx).expect_ok().unwrap());
      auto const expr(boost::get<function<expression>>(fn_expr.data).arities[0].body.body.front());
      auto const *typed_expr(boost::get<function<expression>>(&expr.data));
      CHECK(typed_expr != nullptr);
      CHECK(typed_expr->arities[0].frame->locals.size() == 1);
      CHECK(typed_expr->arities[0].body.body.size() == 1);
      CHECK(typed_expr->arities[0].is_variadic == true);
    }

    SUBCASE("Multiple, single variadic")
    {
      read::lex::processor l_prc{ "(fn* ([a] 1) ([a & args] a))" };
      read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
      processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

      auto const fn_expr(an_prc.result(an_ctx).expect_ok().unwrap());
      auto const expr(boost::get<function<expression>>(fn_expr.data).arities[0].body.body.front());
      auto const *typed_expr(boost::get<function<expression>>(&expr.data));
      CHECK(typed_expr != nullptr);
      CHECK(typed_expr->arities[0].frame->locals.size() == 1);
      CHECK(typed_expr->arities[0].body.body.size() == 1);
      CHECK(typed_expr->arities[0].is_variadic == false);
      CHECK(typed_expr->arities[1].frame->locals.size() == 2);
      CHECK(typed_expr->arities[1].body.body.size() == 1);
      CHECK(typed_expr->arities[1].is_variadic == true);
    }

    SUBCASE("Multiple variadic")
    {
      read::lex::processor l_prc{ "(fn* ([a] 1) ([a & args] 2) ([a b & args] 2))" };
      read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
      processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

      CHECK(an_prc.result(an_ctx).is_err());
    }

    SUBCASE("Fixed arity with same arity as variadic")
    {
      read::lex::processor l_prc{ "(fn* ([a] 1) ([& args] 2))" };
      read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
      processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

      CHECK(an_prc.result(an_ctx).is_err());
    }
  }
}
