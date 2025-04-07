#pragma once

#include <jank/analyze/expression.hpp>

namespace jank::runtime::obj
{
  using persistent_list_ref = jtl::object_ref<struct persistent_list>;
}

namespace jank::analyze::expr
{
  using call_ref = jtl::ref<struct call>;

  struct call : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::call };

    call(expression_position position,
         local_frame_ptr frame,
         native_bool needs_box,
         expression_ref source,
         runtime::obj::persistent_list_ref form,
         native_vector<expression_ref> &&arg_exprs);

    runtime::object_ptr to_runtime_data() const override;

    /* Var, local, or callable. */
    expression_ref source_expr;
    /* We keep the original form from the call expression so we can point
     * back to it if an exception is thrown during eval. */
    runtime::obj::persistent_list_ref form{};
    native_vector<expression_ref> arg_exprs;
  };
}
