#pragma once

#include <jank/analyze/expression_base.hpp>

namespace jank::analyze::expr
{
  using namespace jank::runtime;

  template <typename E>
  struct do_ : expression_base
  {
    native_vector<native_box<E>> values{};

    object_ptr to_runtime_data() const
    {
      object_ptr body_maps{ make_box<obj::persistent_vector>() };
      for(auto const &e : values)
      {
        body_maps = conj(body_maps, e->to_runtime_data());
      }
      return merge(static_cast<expression_base const *>(this)->to_runtime_data(),
                   obj::persistent_array_map::create_unique(make_box("__type"),
                                                            make_box("expr::do"),
                                                            make_box("body"),
                                                            body_maps));
    }
  };
}
