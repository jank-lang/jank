#pragma once

#include <jank/analyze/expression.hpp>

namespace jank::runtime
{
  namespace obj
  {
    using symbol_ref = jtl::object_ref<struct symbol>;
  }

  using var_ref = jtl::object_ref<struct var>;
}

namespace jank::analyze::expr
{
  using var_deref_ref = jtl::ref<struct var_deref>;

  struct var_deref : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::var_deref };

    var_deref(expression_position position,
              local_frame_ptr frame,
              native_bool needs_box,
              runtime::obj::symbol_ref qualified_name,
              runtime::var_ref var);

    runtime::object_ptr to_runtime_data() const override;

    /* Holds the fully qualified name for the originally resolved var.
     * It will be useful to know that the var deref happened through a
     * referred var, for static analysis and error reporting.
     *
     * For all the other purposes, `var` member should be used that points
     * to the actual value of the var.. */
    runtime::obj::symbol_ref qualified_name{};
    runtime::var_ref var{};
  };
}
