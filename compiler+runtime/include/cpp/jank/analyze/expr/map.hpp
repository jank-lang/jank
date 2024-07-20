#pragma once

#include <jank/analyze/expression_base.hpp>

namespace jank::analyze::expr
{
  using namespace jank::runtime;

  template <typename E>
  struct map : expression_base
  {
    native_vector<std::pair<native_box<E>, native_box<E>>> data_exprs;
    option<object_ptr> meta;

    object_ptr to_runtime_data() const
    {
      object_ptr pair_maps(make_box<obj::persistent_vector>());
      for(auto const &e : data_exprs)
      {
        pair_maps = conj(pair_maps,
                         make_box<obj::persistent_vector>(std::in_place,
                                                          e.first->to_runtime_data(),
                                                          e.second->to_runtime_data()));
      }

      return merge(static_cast<expression_base const *>(this)->to_runtime_data(),
                   obj::persistent_array_map::create_unique(make_box("__type"),
                                                            make_box("expr::map"),
                                                            make_box("data_exprs"),
                                                            pair_maps,
                                                            make_box("meta"),
                                                            jank::detail::to_runtime_data(meta)));
    }
  };
}
