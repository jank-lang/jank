#pragma once

#include <jank/analyze/expression.hpp>

namespace jank::runtime::obj
{
  using persistent_list_ref = oref<struct persistent_list>;
}

namespace jank::analyze::expr
{
  using cpp_builtin_operator_call_ref = jtl::ref<struct cpp_builtin_operator_call>;

  struct cpp_builtin_operator_call : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::cpp_builtin_operator_call };

    cpp_builtin_operator_call(expression_position position,
                              local_frame_ptr frame,
                              bool needs_box,
                              int op,
                              native_vector<expression_ref> &&arg_exprs,
                              jtl::ptr<void> type);

    runtime::object_ref to_runtime_data() const override;

    /* Corresponds with Cpp::Operator. */
    int op{};
    native_vector<expression_ref> arg_exprs;
    jtl::ptr<void> type{};
  };
}
