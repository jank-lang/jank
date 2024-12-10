#pragma once

#include <jank/analyze/expression_base.hpp>
#include <jank/detail/to_runtime_data.hpp>

namespace jank::analyze::expr
{
  using namespace jank::runtime;

  template <typename E>
  struct throw_ : expression_base
  {
    native_box<E> value{};

    void propagate_position(expression_position const pos)
    {
      position = pos;
    }

    object_ptr to_runtime_data() const
    {
      return merge(static_cast<expression_base const *>(this)->to_runtime_data(),
                   obj::persistent_array_map::create_unique(make_box("__type"),
                                                            make_box("expr::throw"),
                                                            make_box("value"),
                                                            jank::detail::to_runtime_data(*value)));
    }
  };
}
