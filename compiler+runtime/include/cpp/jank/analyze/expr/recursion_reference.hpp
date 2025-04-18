#pragma once

#include <jank/analyze/expression.hpp>

namespace jank::analyze::expr
{
  using function_context_ref = jtl::ref<struct function_context>;
  using recursion_reference_ref = jtl::ref<struct recursion_reference>;

  struct recursion_reference : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::recursion_reference };

    recursion_reference(expression_position position,
                        local_frame_ptr frame,
                        bool needs_box,
                        function_context_ref fn_ctx);

    runtime::object_ref to_runtime_data() const override;

    function_context_ref fn_ctx;
  };
}
