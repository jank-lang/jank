#pragma once

#include <jank/analyze/expression.hpp>

namespace jank::analyze::expr
{
  using cpp_literal_ref = jtl::ref<struct cpp_literal>;

  struct cpp_literal : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::cpp_literal };

    cpp_literal(expression_position position,
                local_frame_ptr frame,
                bool needs_box,
                runtime::object_ref form,
                runtime::object_ref const data);

    runtime::object_ref to_runtime_data() const override;
    jtl::ptr<void> get_type() const override;

    runtime::object_ref data{};
  };
}
