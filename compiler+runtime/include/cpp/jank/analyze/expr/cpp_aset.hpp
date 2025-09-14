#pragma once

#include <jank/analyze/expression.hpp>

namespace jank::runtime::obj
{
  using persistent_list_ref = oref<struct persistent_list>;
}

namespace jank::analyze::expr
{
  using cpp_aset_ref = jtl::ref<struct cpp_aset>;

  struct cpp_aset : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::cpp_aset };

    cpp_aset(expression_position position,
             local_frame_ptr frame,
             bool needs_box,
             expression_ref array_expr,
             expression_ref index_expr,
             expression_ref value_expr,
             jtl::ptr<void> element_type);

    void propagate_position(expression_position const pos) override;
    runtime::object_ref to_runtime_data() const override;
    void walk(std::function<void(jtl::ref<expression>)> const &f) override;

    expression_ref array_expr;
    expression_ref index_expr;
    expression_ref value_expr;
    jtl::ptr<void> element_type{};
  };
}