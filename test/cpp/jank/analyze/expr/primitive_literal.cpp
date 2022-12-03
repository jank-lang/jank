#include <doctest/doctest.h>

#include <jank/read/lex.hpp>
#include <jank/read/parse.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/analyze/processor.hpp>

namespace jank::analyze::expr
{
  TEST_CASE("Nil")
  {
    read::lex::processor l_prc{ "nil" };
    read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
    runtime::context rt_ctx;
    context an_ctx{ rt_ctx };
    processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

    auto const fn_expr(an_prc.result(an_ctx).expect_ok());
    auto const expr(boost::get<function<expression>>(fn_expr->data).arities[0].body.body.front());
    auto const *typed_expr(boost::get<primitive_literal<expression>>(&expr->data));
    CHECK(typed_expr != nullptr);
    CHECK(typed_expr->data != nullptr);
    CHECK(typed_expr->data->equal(runtime::obj::nil{}));
  }
}
