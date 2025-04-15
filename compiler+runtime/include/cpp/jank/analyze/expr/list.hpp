#pragma once

#include <jtl/option.hpp>

#include <jank/analyze/expression.hpp>

namespace jank::analyze::expr
{
  using list_ref = jtl::ref<struct list>;

  struct list : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::list };

    list(expression_position position,
         local_frame_ptr frame,
         native_bool needs_box,
         native_vector<expression_ref> &&data_exprs,
         jtl::option<runtime::object_ref> const &meta);

    runtime::object_ref to_runtime_data() const override;

    native_vector<expression_ref> data_exprs;
    jtl::option<runtime::object_ref> meta;
  };
}
