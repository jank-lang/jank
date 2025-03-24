#pragma once

#include <jank/analyze/expression.hpp>

namespace jank::runtime::obj
{
  using persistent_list_ptr = native_box<struct persistent_list>;
}

namespace jank::analyze::expr
{
  using call_ptr = runtime::native_box<struct call>;

  struct call : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::call };

    call(expression_position position,
         local_frame_ptr frame,
         native_bool needs_box,
         expression_ptr source,
         runtime::obj::persistent_list_ptr form,
         native_vector<expression_ptr> &&arg_exprs);

    runtime::object_ptr to_runtime_data() const override;

    /* Var, local, or callable. */
    expression_ptr source_expr{};
    /* We keep the original form from the call expression so we can point
     * back to it if an exception is thrown during eval. */
    runtime::obj::persistent_list_ptr form{};
    native_vector<expression_ptr> arg_exprs;
  };
}
