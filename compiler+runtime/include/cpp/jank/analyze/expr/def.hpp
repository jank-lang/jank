#pragma once

#include <jtl/option.hpp>

#include <jank/analyze/expression.hpp>

namespace jank::runtime::obj
{
  using symbol_ref = jtl::object_ref<struct symbol>;
}

namespace jank::analyze::expr
{
  using def_ref = jtl::ref<struct def>;

  struct def : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::def };

    def(expression_position position,
        local_frame_ptr frame,
        native_bool needs_box,
        runtime::obj::symbol_ref name,
        jtl::option<expression_ref> const &value);

    runtime::object_ptr to_runtime_data() const override;

    runtime::obj::symbol_ref name{};
    jtl::option<expression_ref> value;
  };
}
