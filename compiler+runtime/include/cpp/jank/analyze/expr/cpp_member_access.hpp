#pragma once

#include <jank/analyze/expression.hpp>

namespace jank::analyze::expr
{
  using cpp_member_access_ref = jtl::ref<struct cpp_member_access>;

  struct cpp_member_access : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::cpp_member_access };

    cpp_member_access(expression_position position,
                      local_frame_ptr frame,
                      bool needs_box,
                      jtl::ptr<void> type,
                      jtl::ptr<void> scope,
                      jtl::immutable_string const &name,
                      expression_ref obj_expr);

    void propagate_position(expression_position const pos) override;
    runtime::object_ref to_runtime_data() const override;

    /* The type of the member. */
    jtl::ptr<void> type{};
    /* The scope of the member. */
    jtl::ptr<void> scope{};
    jtl::immutable_string name;
    expression_ref obj_expr;
  };
}
