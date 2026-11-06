#pragma once

#include <jtl/option.hpp>
#include <jank/analyze/expression.hpp>

namespace jank::runtime::obj
{
  using symbol_ref = oref<struct symbol>;
}

namespace jank::analyze::expr
{
  using cpp_def_ref = jtl::ref<struct cpp_def>;

  struct cpp_def : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::cpp_def };

    cpp_def(expression_position position,
            local_frame_ptr frame,
            bool needs_box,
            jtl::ptr<void> type,
            runtime::obj::symbol_ref name,
            jtl::option<expression_ref> &value_expr);

    void propagate_position(expression_position const pos) override;
    runtime::object_ref to_runtime_data() const override;
    void walk(std::function<void(jtl::ref<expression>)> const &f) override;

    jtl::ptr<void> type{};
    runtime::obj::symbol_ref name;
    jtl::option<expression_ref> value_expr;
  };
}
