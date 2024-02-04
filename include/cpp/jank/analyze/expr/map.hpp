#pragma once

#include <jank/analyze/expression_base.hpp>
#include <jank/runtime/seq.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct map : expression_base
  {
    native_vector<std::pair<native_box<E>, native_box<E>>> data_exprs;
    option<runtime::object_ptr> meta;

    runtime::object_ptr to_runtime_data() const
    {
      runtime::object_ptr pair_maps(make_box<runtime::obj::persistent_vector>());
      for(auto const &e : data_exprs)
      {
        pair_maps
          = runtime::conj(pair_maps,
                          make_box<runtime::obj::persistent_vector>(std::in_place,
                                                                    e.first->to_runtime_data(),
                                                                    e.second->to_runtime_data()));
      }

      return runtime::merge(
        static_cast<expression_base const *>(this)->to_runtime_data(),
        runtime::obj::persistent_array_map::create_unique(make_box("__type"),
                                                          make_box("expr::map"),
                                                          make_box("data_exprs"),
                                                          pair_maps,
                                                          make_box("meta"),
                                                          detail::to_runtime_data(meta)));
    }
  };
}
