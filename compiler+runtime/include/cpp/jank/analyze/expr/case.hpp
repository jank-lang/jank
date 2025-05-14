#pragma once

#include <jank/analyze/expression.hpp>

namespace jank::analyze::expr
{
  using namespace jank::runtime;

  using case_ref = jtl::ref<struct case_>;

  struct case_ : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::case_ };

    case_(expression_position position,
          local_frame_ptr frame,
          bool needs_box,
          expression_ref value_expr,
          i64 shift,
          i64 mask,
          expression_ref default_expr,
          native_vector<i64> &&keys,
          native_vector<expression_ref> &&exprs);

    void propagate_position(expression_position const pos) override;
    runtime::object_ref to_runtime_data() const override;

    expression_ref value_expr;
    i64 shift{};
    i64 mask{};
    expression_ref default_expr;
    native_vector<i64> keys{};
    native_vector<expression_ref> exprs{};
  };
}
