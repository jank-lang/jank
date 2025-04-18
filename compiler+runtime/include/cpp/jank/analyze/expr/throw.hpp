#pragma once

#include <jank/analyze/expression.hpp>

namespace jank::analyze::expr
{
  using throw_ref = jtl::ref<struct throw_>;

  struct throw_ : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::throw_ };

    throw_(expression_position position,
           local_frame_ptr frame,
           bool needs_box,
           expression_ref value);

    runtime::object_ref to_runtime_data() const override;

    expression_ref value;
  };
}
