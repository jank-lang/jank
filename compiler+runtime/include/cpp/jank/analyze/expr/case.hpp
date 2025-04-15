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
          native_bool needs_box,
          expression_ref value_expr,
          native_integer shift,
          native_integer mask,
          expression_ref default_expr,
          native_vector<native_integer> &&keys,
          native_vector<expression_ref> &&exprs);

    void propagate_position(expression_position const pos) override;
    runtime::object_ref to_runtime_data() const override;

    expression_ref value_expr;
    native_integer shift{};
    native_integer mask{};
    expression_ref default_expr;
    native_vector<native_integer> keys{};
    native_vector<expression_ref> exprs{};
  };
}
