#pragma once

#include <jank/analyze/expression.hpp>

namespace jank::analyze::expr
{
  using set_ref = jtl::ref<struct set>;

  struct set : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::set };

    set(expression_position position,
        local_frame_ptr frame,
        bool needs_box,
        native_vector<expression_ref> &&data_exprs,
        runtime::object_ref const meta);

    runtime::object_ref to_runtime_data() const override;
    void walk(std::function<void(jtl::ref<expression>)> const &f) override;

    native_vector<expression_ref> data_exprs;
    runtime::object_ref meta;
  };
}
