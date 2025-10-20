#pragma once

#include <jank/read/source.hpp>
#include <jank/analyze/expression.hpp>

namespace jank::analyze::expr
{
  using cpp_unbox_ref = jtl::ref<struct cpp_unbox>;

  struct cpp_unbox : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::cpp_unbox };

    cpp_unbox(expression_position position,
              local_frame_ptr frame,
              bool needs_box,
              jtl::ptr<void> type,
              expression_ref value_expr,
              read::source const &source);

    void propagate_position(expression_position const pos) override;
    runtime::object_ref to_runtime_data() const override;
    void walk(std::function<void(jtl::ref<expression>)> const &f) override;

    jtl::ptr<void> type{};
    expression_ref value_expr;
    read::source source;
  };
}
