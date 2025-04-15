#pragma once

#include <jank/analyze/expression.hpp>

namespace jank::runtime
{
  namespace obj
  {
    using symbol_ref = jtl::oref<struct symbol>;
  }

  using var_ref = jtl::oref<struct var>;
}

namespace jank::analyze::expr
{
  using var_ref_ref = jtl::ref<struct var_ref>;

  struct var_ref : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::var_ref };

    var_ref(expression_position position,
            local_frame_ptr frame,
            native_bool needs_box,
            runtime::obj::symbol_ref qualified_name,
            runtime::var_ref var);

    runtime::object_ref to_runtime_data() const override;

    runtime::obj::symbol_ref qualified_name{};
    runtime::var_ref var{};
  };
}
