#pragma once

#include <jtl/option.hpp>

#include <jank/analyze/expression.hpp>

namespace jank::analyze::expr
{
  using throw_ref = jtl::ref<struct throw_>;

  struct throw_ : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::throw_ };

    throw_(expression_position position,
           local_frame_ptr frame,
           bool needs_box,
           runtime::object_ref form);
    throw_(expression_position position,
           local_frame_ptr frame,
           bool needs_box,
           runtime::object_ref form,
           expression_ref value);

    runtime::object_ref to_runtime_data() const override;
    void walk(std::function<void(jtl::ref<expression>)> const &f) override;

    /* TODO: Rename to value_expr. */
    /* A throw without a value will re-throw the current exception. If there is no current
     * exception, the program will terminate. */
    jtl::option<expression_ref> value;
  };
}
