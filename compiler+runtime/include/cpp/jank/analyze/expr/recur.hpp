#pragma once

#include <jank/analyze/expression.hpp>

namespace jank::runtime::obj
{
  using persistent_list_ptr = native_box<struct persistent_list>;
}

namespace jank::analyze::expr
{
  using recur_ptr = jtl::ref<struct recur>;

  struct recur : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::recur };

    recur(expression_position position,
          local_frame_ptr frame,
          native_bool needs_box,
          runtime::obj::persistent_list_ptr args,
          native_vector<expression_ptr> &&arg_exprs);

    runtime::object_ptr to_runtime_data() const override;

    runtime::obj::persistent_list_ptr args{};
    native_vector<expression_ptr> arg_exprs;
  };
}
