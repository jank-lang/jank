#pragma once

#include <jank/analyze/expression.hpp>

namespace jank::runtime::obj
{
  using symbol_ptr = runtime::native_box<struct symbol>;
}

namespace jank::analyze
{
  using local_binding_ptr = runtime::native_box<struct local_binding>;
}

namespace jank::analyze::expr
{
  using local_reference_ptr = runtime::native_box<struct local_reference>;

  struct local_reference : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::local_reference };

    local_reference(expression_position position,
                    local_frame_ptr frame,
                    native_bool needs_box,
                    runtime::obj::symbol_ptr name,
                    local_binding_ptr binding);

    runtime::object_ptr to_runtime_data() const override;

    runtime::obj::symbol_ptr name{};
    local_binding_ptr binding{};
  };
}
