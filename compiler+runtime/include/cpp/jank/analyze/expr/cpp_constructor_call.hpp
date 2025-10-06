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
                         jtl::ptr<void> fn,
                         bool is_aggregate,
                         native_vector<expression_ref> &&arg_exprs);

    void propagate_position(expression_position const pos) override;
    runtime::object_ref to_runtime_data() const override;
    void walk(std::function<void(jtl::ref<expression>)> const &f) override;

    /* The type we're constructing. */
    jtl::ptr<void> type{};
    /* The matched ctor overload to call. If our type is builtin, this will be null.
     *
     * Also, if we're doing aggregate initialization, this will be null and `is_aggregate`
     * will be true.*/
    jtl::ptr<void> fn{};
    native_vector<expression_ref> arg_exprs;
    bool is_aggregate{};
  };
}
