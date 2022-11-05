#include <doctest/doctest.h>

#include <jank/read/lex.hpp>
#include <jank/read/parse.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/obj/map.hpp>
#include <jank/analyze/processor.hpp>

namespace jank::analyze::expr
{
  TEST_CASE("Map")
  {
    read::lex::processor l_prc{ "{:foo true :bar 0}" };
    read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
    runtime::context rt_ctx;
    context an_ctx{ rt_ctx };
    processor an_prc{ rt_ctx, p_prc.begin(), p_prc.end() };

    auto const fn_expr(an_prc.result(an_ctx).expect_ok().unwrap());
    auto const expr(boost::get<function<expression>>(fn_expr.data).body.body.front());
    auto const *typed_expr(boost::get<map<expression>>(&expr.data));
    CHECK(typed_expr != nullptr);
    CHECK(typed_expr->data_exprs.size() == 2);

    CHECK
    (
      boost::get<primitive_literal<expression>>(typed_expr->data_exprs[0].first.data).data->equal
      (
        runtime::make_box<runtime::obj::keyword>(runtime::obj::symbol{ "foo" }, true)
      )
    );
    CHECK
    (
      boost::get<primitive_literal<expression>>(typed_expr->data_exprs[0].second.data).data->equal
      (
        runtime::make_box<runtime::obj::boolean>(true)
      )
    );
    CHECK
    (
      boost::get<primitive_literal<expression>>(typed_expr->data_exprs[1].first.data).data->equal
      (
        runtime::make_box<runtime::obj::keyword>(runtime::obj::symbol{ "bar" }, true)
      )
    );
    CHECK
    (
      boost::get<primitive_literal<expression>>(typed_expr->data_exprs[1].second.data).data->equal
      (
        runtime::make_box<runtime::obj::integer>(0)
      )
    );
  }
}
