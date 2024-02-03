#pragma once

#include <jank/analyze/expression_base.hpp>
#include <jank/detail/to_runtime_data.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct throw_ : expression_base
  {
    native_box<E> value{};

    runtime::object_ptr to_runtime_data() const
    {
      return runtime::merge(
        static_cast<expression_base const *>(this)->to_runtime_data(),
        runtime::obj::persistent_array_map::create_unique(make_box("__type"),
                                                          make_box("expr::throw"),
                                                          make_box("value"),
                                                          detail::to_runtime_data(*value)));
    }
  };
}
