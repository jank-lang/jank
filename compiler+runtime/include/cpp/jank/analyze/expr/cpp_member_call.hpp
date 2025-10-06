#pragma once

#include <jank/analyze/expression.hpp>

namespace jank::analyze::expr
{
  using cpp_member_call_ref = jtl::ref<struct cpp_member_call>;

  struct cpp_member_call : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::cpp_member_call };

    cpp_member_call(expression_position position,
                    local_frame_ptr frame,
                    bool needs_box,
                    jtl::ptr<void> type,
                    jtl::ptr<void> fn,
                    native_vector<expression_ref> &&arg_exprs);

    void propagate_position(expression_position const pos) override;
    runtime::object_ref to_runtime_data() const override;
    void walk(std::function<void(jtl::ref<expression>)> const &f) override;

    /* The type of the member. */
    jtl::ptr<void> parent_type{};
    /* The matched fn to call. */
    jtl::ptr<void> fn{};
    native_vector<expression_ref> arg_exprs;
  };
}
