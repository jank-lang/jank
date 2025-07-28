#pragma once

#include <jank/analyze/expression.hpp>

namespace jank::runtime::obj
{
  using symbol_ref = oref<struct symbol>;
}

namespace jank::analyze::expr
{
  using cpp_type_ref = jtl::ref<struct cpp_type>;

  struct cpp_type : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::cpp_type };

    cpp_type();
    cpp_type(expression_position position,
             local_frame_ptr frame,
             bool needs_box,
             runtime::obj::symbol_ref sym,
             jtl::ptr<void> type);

    void propagate_position(expression_position const pos) override;
    runtime::object_ref to_runtime_data() const override;

    runtime::obj::symbol_ref sym;
    jtl::ptr<void> type{};
  };
}
