#pragma once

#include <jtl/option.hpp>

#include <jank/analyze/expression.hpp>

namespace jank::analyze::expr
{
  using if_ptr = jtl::ref<struct if_>;

  struct if_ : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::if_ };

    if_(expression_position position,
        local_frame_ptr frame,
        native_bool needs_box,
        expression_ptr condition,
        expression_ptr then,
        jtl::option<expression_ptr> const &else_);

    void propagate_position(expression_position const pos) override;
    runtime::object_ptr to_runtime_data() const override;

    expression_ptr condition;
    expression_ptr then;
    jtl::option<expression_ptr> else_;
  };
}
