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
  using var_ref_ref = jtl::ref<struct var_ref>;

  struct var_ref : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::var_ref };

    var_ref(expression_position position,
            local_frame_ptr frame,
            native_bool needs_box,
            runtime::obj::symbol_ptr qualified_name,
            runtime::var_ptr var);

    runtime::object_ptr to_runtime_data() const override;

    runtime::obj::symbol_ptr qualified_name{};
    runtime::var_ptr var{};
  };
}
