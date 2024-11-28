#pragma once

#include <jank/analyze/expression_base.hpp>
#include <jank/runtime/obj/persistent_hash_set.hpp>

namespace jank::analyze::expr
{
  using namespace jank::runtime;

  template <typename E>
  struct set : expression_base
  {
    native_vector<native_box<E>> data_exprs;
    option<object_ptr> meta;
    object_ptr data{};

    void propagate_position(expression_position const pos)
    {
      position = pos;
    }

    object_ptr to_runtime_data() const
    {
      object_ptr pair_maps(make_box<obj::persistent_vector>());
      for(auto const &e : data_exprs)
      {
        pair_maps = conj(pair_maps, e->to_runtime_data());
      }

      return merge(static_cast<expression_base const *>(this)->to_runtime_data(),
                   obj::persistent_array_map::create_unique(make_box("__type"),
                                                            make_box("expr::set"),
                                                            make_box("data_exprs"),
                                                            pair_maps,
                                                            make_box("meta"),
                                                            jank::detail::to_runtime_data(meta)));
    }
  };
}
