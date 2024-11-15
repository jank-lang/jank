#pragma once

#include <jank/analyze/expression_base.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct recursion_reference : expression_base
  {
    function_context_ptr fn_ctx{};

    void propagate_position(expression_position const pos)
    {
      position = pos;
    }

    runtime::object_ptr to_runtime_data() const
    {
      return merge(
        static_cast<expression_base const *>(this)->to_runtime_data(),
        runtime::obj::persistent_array_map::create_unique(make_box("__type"),
                                                          make_box("expr::recursion_reference")));
    }
  };
}
