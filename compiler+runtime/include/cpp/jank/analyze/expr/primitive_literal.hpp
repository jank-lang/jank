#pragma once

#include <jank/analyze/local_frame.hpp>
#include <jank/analyze/expression_base.hpp>
#include <jank/detail/to_runtime_data.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct primitive_literal : expression_base
  {
    runtime::object_ptr data{};

    void propagate_position(expression_position const pos)
    {
      position = pos;
    }

    runtime::object_ptr to_runtime_data() const
    {
      using namespace runtime::obj;
      return runtime::merge(static_cast<expression_base const *>(this)->to_runtime_data(),
                            persistent_array_map::create_unique(make_box("__type"),
                                                                make_box("expr::primitive_literal"),
                                                                make_box("data"),
                                                                data));
    }
  };
}
