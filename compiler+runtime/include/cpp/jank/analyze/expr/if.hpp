#pragma once

#include <jtl/option.hpp>

#include <jank/analyze/expression.hpp>

namespace jank::analyze::expr
{
  using if_ref = jtl::ref<struct if_>;

  struct if_ : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::if_ };

    if_(expression_position position,
        local_frame_ptr frame,
        bool needs_box,
        expression_ref condition,
        expression_ref then,
        jtl::option<expression_ref> const &else_);

    void propagate_position(expression_position const pos) override;
    runtime::object_ref to_runtime_data() const override;

    expression_ref condition;
    expression_ref then;
    jtl::option<expression_ref> else_;
  };
}
