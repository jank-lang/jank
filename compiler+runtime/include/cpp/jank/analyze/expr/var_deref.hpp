#pragma once

#include <jank/runtime/var.hpp>
#include <jank/analyze/local_frame.hpp>
#include <jank/analyze/expression_base.hpp>
#include <jank/detail/to_runtime_data.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct var_deref : expression_base
  {
    /* Holds the fully qualified name for the originally resolved var.
     * It will be useful to know that the var deref happened through a
     * referred var, for static analysis and error reporting.
     *
     * For all the other purposes, `var` member should be used that points
     * to the actual value of the var.. */
    runtime::obj::symbol_ptr qualified_name{};
    runtime::var_ptr var{};

    void propagate_position(expression_position const pos)
    {
      position = pos;
    }

    runtime::object_ptr to_runtime_data() const
    {
      return runtime::merge(
        static_cast<expression_base const *>(this)->to_runtime_data(),
        runtime::obj::persistent_array_map::create_unique(make_box("__type"),
                                                          make_box("expr::var_deref"),
                                                          make_box("qualified_name"),
                                                          qualified_name,
                                                          make_box("var"),
                                                          var));
    }
  };
}
