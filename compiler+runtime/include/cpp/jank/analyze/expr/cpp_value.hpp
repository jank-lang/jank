#pragma once

#include <jank/analyze/expression.hpp>

namespace jank::analyze::expr
{
  using cpp_value_ref = jtl::ref<struct cpp_value>;

  struct cpp_value : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::cpp_value };

    enum class value_kind : u8
    {
      constructor,
      variable,
      enum_constant,
      function
    };

    cpp_value(expression_position position,
              local_frame_ptr frame,
              bool needs_box,
              jtl::ptr<void> type,
              jtl::ptr<void> scope,
              value_kind val_kind);

    void propagate_position(expression_position const pos) override;
    runtime::object_ref to_runtime_data() const override;

    /* The type we're going to construct. */
    jtl::ptr<void> type;
    /* If the type is builtin, the scope will be null. */
    jtl::ptr<void> scope;
    value_kind val_kind;
  };
}
