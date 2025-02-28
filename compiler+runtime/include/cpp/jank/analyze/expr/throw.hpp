#pragma once

#include <jank/analyze/expression.hpp>

namespace jank::analyze::expr
{
  using throw_ptr = runtime::native_box<struct throw_>;

  struct throw_ : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::throw_ };

    throw_(expression_position position,
           local_frame_ptr frame,
           native_bool needs_box,
           expression_ptr value);

    runtime::object_ptr to_runtime_data() const override;

    expression_ptr value{};
  };
}
