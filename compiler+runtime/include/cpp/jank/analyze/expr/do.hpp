#pragma once

#include <jank/analyze/expression.hpp>

namespace jank::analyze::expr
{
  using do_ptr = jtl::ref<struct do_>;

  struct do_ : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::do_ };

    do_();
    do_(expression_position position, local_frame_ptr frame, native_bool needs_box);
    do_(expression_position position,
        local_frame_ptr frame,
        native_bool needs_box,
        native_vector<expression_ptr> &&values);

    void propagate_position(expression_position const pos) override;
    runtime::object_ptr to_runtime_data() const override;

    native_vector<expression_ptr> values{};
  };
}
