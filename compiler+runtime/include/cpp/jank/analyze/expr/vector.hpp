#pragma once

#include <jank/analyze/expression_base.hpp>

namespace jank::analyze::expr
{
  using namespace jank::runtime;

  template <typename E>
  struct vector : expression_base
  {
    native_vector<native_box<E>> data_exprs;
    option<object_ptr> meta;

    void propagate_position(expression_position const pos)
    {
      position = pos;
    }

    object_ptr to_runtime_data() const
    {
      object_ptr exprs(make_box<obj::persistent_vector>());
      for(auto const &e : data_exprs)
      {
        exprs = conj(exprs, e->to_runtime_data());
      }

      return merge(static_cast<expression_base const *>(this)->to_runtime_data(),
                   obj::persistent_array_map::create_unique(make_box("__type"),
                                                            make_box("expr::vector"),
                                                            make_box("data_exprs"),
                                                            exprs,
                                                            make_box("meta"),
                                                            jank::detail::to_runtime_data(meta)));
    }
  };
}
