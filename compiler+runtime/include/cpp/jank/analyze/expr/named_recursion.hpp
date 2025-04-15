#pragma once

#include <jank/analyze/expr/recursion_reference.hpp>

namespace jank::runtime::obj
{
  using persistent_list_ref = oref<struct persistent_list>;
}

namespace jank::analyze::expr
{
  using named_recursion_ref = jtl::ref<struct named_recursion>;

  struct named_recursion : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::named_recursion };

    named_recursion(expression_position position,
                    local_frame_ptr frame,
                    native_bool needs_box,
                    recursion_reference &&recursion_ref,
                    runtime::obj::persistent_list_ref args,
                    native_vector<expression_ref> &&arg_exprs);

    runtime::object_ref to_runtime_data() const override;

    recursion_reference recursion_ref;
    runtime::obj::persistent_list_ref args{};
    native_vector<expression_ref> arg_exprs;
  };
}
