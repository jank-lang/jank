#pragma once

#include <jank/analyze/expression.hpp>

namespace jank::analyze::expr
{
  using cpp_constructor_call_ref = jtl::ref<struct cpp_constructor_call>;

  struct cpp_constructor_call : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::cpp_constructor_call };

    cpp_constructor_call();
    cpp_constructor_call(expression_position position,
                         local_frame_ptr frame,
                         bool needs_box,
                         jtl::ptr<void> type,
                         native_vector<expression_ref> &&args);

    void propagate_position(expression_position const pos) override;
    runtime::object_ref to_runtime_data() const override;

    jtl::ptr<void> type{};
    native_vector<expression_ref> args;
  };
}
