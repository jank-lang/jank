#pragma once

#include <jank/analyze/expression.hpp>

namespace jank::analyze::expr
{
  using cpp_call_ref = jtl::ref<struct cpp_call>;

  struct cpp_call : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::cpp_call };

    cpp_call(expression_position position,
             local_frame_ptr frame,
             bool needs_box,
             jtl::ptr<void> type,
             expression_ref source_expr,
             native_vector<expression_ref> &&arg_exprs);
    cpp_call(expression_position position,
             local_frame_ptr frame,
             bool needs_box,
             jtl::ptr<void> type,
             expression_ref source_expr,
             native_vector<expression_ref> &&arg_exprs,
             jtl::immutable_string const &function_code);

    void propagate_position(expression_position const pos) override;
    runtime::object_ref to_runtime_data() const override;
    void walk(std::function<void(jtl::ref<expression>)> const &f) override;

    /* The return type of the call. */
    jtl::ptr<void> type{};
    expression_ref source_expr;
    native_vector<expression_ref> arg_exprs;
    /* Literals from `cpp/value` will have code which needs to be compiled into the
     * IR module. We hang onto it here. Most of the time, this will be empty. */
    jtl::immutable_string function_code;
  };
}
