#pragma once

#include <jank/analyze/expression.hpp>

namespace jank::analyze::expr
{
  using function_context_ptr = runtime::native_box<struct function_context>;
  using recursion_reference_ptr = runtime::native_box<struct recursion_reference>;

  struct recursion_reference : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::recursion_reference };

    recursion_reference();
    recursion_reference(expression_position position,
                        local_frame_ptr frame,
                        native_bool needs_box,
                        function_context_ptr fn_ctx);

    runtime::object_ptr to_runtime_data() const override;

    function_context_ptr fn_ctx{};
  };
}
