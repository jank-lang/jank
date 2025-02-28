#pragma once

#include <jank/analyze/expression.hpp>

namespace jank::runtime
{
  namespace obj
  {
    using symbol_ptr = native_box<struct symbol>;
  }

  using var_ptr = native_box<struct var>;
}

namespace jank::analyze::expr
{
  using var_deref_ptr = runtime::native_box<struct var_deref>;

  struct var_deref : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::var_deref };

    var_deref(expression_position position,
              local_frame_ptr frame,
              native_bool needs_box,
              runtime::obj::symbol_ptr qualified_name,
              runtime::var_ptr var);

    runtime::object_ptr to_runtime_data() const override;

    /* Holds the fully qualified name for the originally resolved var.
     * It will be useful to know that the var deref happened through a
     * referred var, for static analysis and error reporting.
     *
     * For all the other purposes, `var` member should be used that points
     * to the actual value of the var.. */
    runtime::obj::symbol_ptr qualified_name{};
    runtime::var_ptr var{};
  };
}
