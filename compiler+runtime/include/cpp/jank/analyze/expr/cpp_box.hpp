#pragma once

#include <jank/analyze/expression.hpp>

namespace jank::runtime::obj
{
  using symbol_ref = oref<struct symbol>;
}

namespace jank::analyze::expr
{
  using cpp_box_ref = jtl::ref<struct cpp_box>;

  struct cpp_box : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::cpp_box };

    cpp_box(expression_position position,
            local_frame_ptr frame,
            bool needs_box,
            expression_ref value_expr);

    void propagate_position(expression_position const pos) override;
    runtime::object_ref to_runtime_data() const override;

    expression_ref value_expr;
  };
}
