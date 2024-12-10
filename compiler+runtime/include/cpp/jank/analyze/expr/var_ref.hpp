#pragma once

#include <jank/runtime/var.hpp>
#include <jank/analyze/local_frame.hpp>
#include <jank/analyze/expression_base.hpp>
#include <jank/detail/to_runtime_data.hpp>

namespace jank::analyze::expr
{
  using namespace jank::runtime;

  template <typename E>
  struct var_ref : expression_base
  {
    obj::symbol_ptr qualified_name{};
    var_ptr var{};

    void propagate_position(expression_position const pos)
    {
      position = pos;
    }

    object_ptr to_runtime_data() const
    {
      return merge(static_cast<expression_base const *>(this)->to_runtime_data(),
                   obj::persistent_array_map::create_unique(make_box("__type"),
                                                            make_box("expr::var_ref"),
                                                            make_box("qualified_name"),
                                                            qualified_name,
                                                            make_box("var"),
                                                            var));
    }
  };
}
