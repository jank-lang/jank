#pragma once

#include <jank/analyze/expression.hpp>

namespace jank::analyze::expr
{
  using cpp_call_ref = jtl::ref<struct cpp_call>;

  struct cpp_call : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::cpp_call };

    cpp_call();
    cpp_call(expression_position position,
             local_frame_ptr frame,
             bool needs_box,
             jtl::ptr<void> type,
             jtl::ptr<void> fn,
             native_vector<expression_ref> &&arg_exprs);

    void propagate_position(expression_position const pos) override;
    runtime::object_ref to_runtime_data() const override;

    /* The return type of the call. */
    jtl::ptr<void> type{};
    /* The matched function overload to call. */
    jtl::ptr<void> fn{};
    native_vector<expression_ref> arg_exprs;
  };
}
