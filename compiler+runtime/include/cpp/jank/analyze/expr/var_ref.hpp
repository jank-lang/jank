#pragma once

#include <jank/analyze/expression.hpp>

namespace jank::runtime
{
  namespace obj
  {
    using symbol_ref = oref<struct symbol>;
  }

  using var_ref = oref<struct var>;
}

namespace jank::analyze::expr
{
  struct var_ref : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::var_ref };

    var_ref(expression_position position,
            local_frame_ptr frame,
            bool needs_box,
            runtime::obj::symbol_ref const qualified_name,
            runtime::var_ref const var);

    runtime::object_ref to_runtime_data() const override;

    /* Holds the fully qualified name for the originally resolved var.
     * It will be useful to know that the var ref happened through a
     * referred var, for static analysis and error reporting.
     *
     * For all the other purposes, `var` member should be used that points
     * to the actual value of the var.. */
    runtime::obj::symbol_ref qualified_name{};
    runtime::var_ref var{};
  };

  using var_ref_ref = jtl::ref<var_ref>;
}
