#pragma once

#include <jank/analyze/expression.hpp>

namespace jank::analyze::expr
{
  using cpp_unsafe_cast_ref = jtl::ref<struct cpp_unsafe_cast>;

  /* Unsafe cast expressions are the equivalent of C++'s C-style casting. This supports
   * normal explicit casts as well as the following cases:
   *
   * 1. integer -> pointer to anything
   * 2. pointer to anything -> integer
   * 3. pointer to object -> pointer to object
   * 4. pointer to function -> pointer to function
   *
   * Additionally, unsafe casts support adding/removing const. */
  struct cpp_unsafe_cast : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::cpp_unsafe_cast };

    cpp_unsafe_cast(expression_position position,
                    local_frame_ptr frame,
                    bool needs_box,
                    jtl::ptr<void> type,
                    expression_ref value_expr);

    void propagate_position(expression_position const pos) override;
    runtime::object_ref to_runtime_data() const override;
    void walk(std::function<void(jtl::ref<expression>)> const &f) override;

    /* This is the resulting type of the conversion. */
    jtl::ptr<void> type{};
    expression_ref value_expr;
  };
}
