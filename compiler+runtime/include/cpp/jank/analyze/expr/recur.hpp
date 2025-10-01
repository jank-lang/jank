#pragma once

#include <jtl/option.hpp>

#include <jank/analyze/expression.hpp>

namespace jank::runtime::obj
{
  using persistent_list_ref = oref<struct persistent_list>;
}

namespace jank::analyze::expr
{
  using recur_ref = jtl::ref<struct recur>;
  using let_ref = jtl::ref<struct let>;

  struct recur : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::recur };

    recur(expression_position position,
          local_frame_ptr frame,
          bool needs_box,
          runtime::obj::persistent_list_ref args,
          native_vector<expression_ref> &&arg_exprs,
          jtl::option<let_ref> const &loop_target);

    runtime::object_ref to_runtime_data() const override;
    void walk(std::function<void(jtl::ref<expression>)> const &f) override;

    runtime::obj::persistent_list_ref args{};
    native_vector<expression_ref> arg_exprs;
    jtl::option<let_ref> loop_target;
  };
}
