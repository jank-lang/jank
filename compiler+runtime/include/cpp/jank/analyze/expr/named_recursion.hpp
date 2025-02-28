#pragma once

#include <jank/analyze/expr/recursion_reference.hpp>

namespace jank::runtime::obj
{
  using persistent_list_ptr = native_box<struct persistent_list>;
}

namespace jank::analyze::expr
{
  using named_recursion_ptr = runtime::native_box<struct named_recursion>;

  struct named_recursion : expression
  {
    static constexpr expression_kind expr_kind{ expression_kind::named_recursion };

    named_recursion(expression_position position,
                    local_frame_ptr frame,
                    native_bool needs_box,
                    recursion_reference &&recursion_ref,
                    runtime::obj::persistent_list_ptr args,
                    native_vector<expression_ptr> &&arg_exprs);

    runtime::object_ptr to_runtime_data() const override;

    recursion_reference recursion_ref{};
    runtime::obj::persistent_list_ptr args{};
    native_vector<expression_ptr> arg_exprs;
  };
}
