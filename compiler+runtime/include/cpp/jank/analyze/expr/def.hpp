#pragma once

#include <jank/analyze/expression.hpp>
#include <jank/option.hpp>

namespace jank::runtime::obj
{
  using symbol_ptr = native_box<struct symbol>;
}

namespace jank::analyze::expr
{
  using def_ptr = jtl::ref<struct def>;

  struct def : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::def };

    def(expression_position position,
        local_frame_ptr frame,
        native_bool needs_box,
        runtime::obj::symbol_ptr name,
        option<expression_ptr> const &value);

    runtime::object_ptr to_runtime_data() const override;

    runtime::obj::symbol_ptr name{};
    option<expression_ptr> value;
  };
}
