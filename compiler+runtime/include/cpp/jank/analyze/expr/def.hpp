#pragma once

#include <jank/runtime/obj/symbol.hpp>
#include <jank/analyze/local_frame.hpp>
#include <jank/analyze/expression_base.hpp>
#include <jank/option.hpp>
#include <jank/detail/to_runtime_data.hpp>

namespace jank::analyze::expr
{
  using namespace jank::runtime;

  template <typename E>
  struct def : expression_base
  {
    obj::symbol_ptr name{};
    option<native_box<E>> value;

    void propagate_position(expression_position const pos)
    {
      position = pos;
    }

    object_ptr to_runtime_data() const
    {
      return merge(static_cast<expression_base const *>(this)->to_runtime_data(),
                   obj::persistent_array_map::create_unique(make_box("__type"),
                                                            make_box("expr::def"),
                                                            make_box("name"),
                                                            name,
                                                            make_box("value"),
                                                            jank::detail::to_runtime_data(value)));
    }
  };
}
