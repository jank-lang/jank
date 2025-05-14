#pragma once

#include <jank/analyze/expression.hpp>

namespace jank::analyze::expr
{
  using do_ref = jtl::ref<struct do_>;

  struct do_ : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::do_ };

    do_();
    do_(expression_position position, local_frame_ptr frame, bool needs_box);
    do_(expression_position position,
        local_frame_ptr frame,
        bool needs_box,
        native_vector<expression_ref> &&values);

    void propagate_position(expression_position const pos) override;
    runtime::object_ref to_runtime_data() const override;

    native_vector<expression_ref> values{};
  };
}
