#pragma once

#include <jank/analyze/expression.hpp>

namespace jank::runtime::obj
{
  using persistent_list_ref = oref<struct persistent_list>;
}

namespace jank::analyze::expr
{
  using recur_ref = jtl::ref<struct recur>;

  struct recur : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::recur };

    recur(expression_position position,
          local_frame_ptr frame,
          native_bool needs_box,
          runtime::obj::persistent_list_ref args,
          native_vector<expression_ref> &&arg_exprs);

    runtime::object_ref to_runtime_data() const override;

    runtime::obj::persistent_list_ref args{};
    native_vector<expression_ref> arg_exprs;
  };
}
