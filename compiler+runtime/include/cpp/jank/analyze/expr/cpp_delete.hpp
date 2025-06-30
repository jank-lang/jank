#pragma once

#include <jank/analyze/expression.hpp>

namespace jank::analyze::expr
{
  using cpp_delete_ref = jtl::ref<struct cpp_delete>;

  struct cpp_delete : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::cpp_delete };

    cpp_delete(expression_position position,
               local_frame_ptr frame,
               bool needs_box,
               expression_ref value_expr);

    void propagate_position(expression_position const pos) override;
    runtime::object_ref to_runtime_data() const override;

    expression_ref value_expr;
  };
}
