#pragma once

#include <jtl/option.hpp>

#include <jank/analyze/expression.hpp>

namespace jank::analyze::expr
{
  using map_ptr = jtl::ref<struct map>;

  struct map : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::map };

    map(expression_position position,
        local_frame_ptr frame,
        native_bool needs_box,
        native_vector<std::pair<expression_ptr, expression_ptr>> &&data_exprs,
        jtl::option<runtime::object_ptr> const &meta);

    runtime::object_ptr to_runtime_data() const override;

    native_vector<std::pair<expression_ptr, expression_ptr>> data_exprs;
    jtl::option<runtime::object_ptr> meta;
  };
}
