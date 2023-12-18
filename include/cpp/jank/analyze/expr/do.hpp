#pragma once

#include <jank/analyze/expression_base.hpp>
#include <jank/runtime/seq.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct do_ : expression_base
  {
    native_vector<native_box<E>> body{};

    runtime::object_ptr to_runtime_data() const
    {
      runtime::object_ptr body_maps(make_box<runtime::obj::vector>());
      for(auto const &e : body)
      { body_maps = runtime::conj(body_maps, e->to_runtime_data()); }

      return runtime::obj::persistent_array_map::create_unique
      (
        make_box("__type"), make_box("expr::do"),
        make_box("body"), body_maps
      );
    }
  };
}
