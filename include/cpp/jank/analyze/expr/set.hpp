#pragma once

#include <jank/analyze/expression_base.hpp>
#include <jank/runtime/seq.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct set : expression_base
  {
    native_vector<native_box<E>> data_exprs;

    runtime::object_ptr to_runtime_data() const
    {
      runtime::object_ptr pair_maps(make_box<runtime::obj::persistent_vector>());
      for(auto const &e : data_exprs)
      {
        pair_maps = runtime::conj(pair_maps, e->to_runtime_data());
      }

      return runtime::merge(
        static_cast<expression_base const *>(this)->to_runtime_data(),
        runtime::obj::persistent_array_map::create_unique(make_box("__type"),
                                                          make_box("expr::set"),
                                                          make_box("data_exprs"),
                                                          pair_maps));
    }
  };
}
