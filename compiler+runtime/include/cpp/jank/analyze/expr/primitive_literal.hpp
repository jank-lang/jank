#pragma once

#include <jank/analyze/expression.hpp>

namespace jank::analyze::expr
{
  using primitive_literal_ref = jtl::ref<struct primitive_literal>;

  struct primitive_literal : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::primitive_literal };

    primitive_literal(expression_position position,
                      local_frame_ptr frame,
                      native_bool needs_box,
                      runtime::object_ptr data);

    runtime::object_ptr to_runtime_data() const override;

    runtime::object_ptr data{};
  };
}
