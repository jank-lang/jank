#pragma once

#include <jank/analyze/expression.hpp>

namespace jank::analyze::expr
{
  using cpp_new_ref = jtl::ref<struct cpp_new>;

  struct cpp_new : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::cpp_new };

    cpp_new(expression_position position,
              local_frame_ptr frame,
              bool needs_box,
              jtl::ptr<void> type,
              expression_ref value_expr);

    void propagate_position(expression_position const pos) override;
    runtime::object_ref to_runtime_data() const override;

    jtl::ptr<void> type{};
    expression_ref value_expr;
  };
}
