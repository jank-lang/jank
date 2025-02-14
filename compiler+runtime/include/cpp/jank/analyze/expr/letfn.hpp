#pragma once

#include <jank/analyze/expr/do.hpp>
#include <jank/analyze/local_frame.hpp>
#include <jank/analyze/expression_base.hpp>
#include <jank/detail/to_runtime_data.hpp>

namespace jank::analyze::expr
{
  using namespace jank::runtime;

  template <typename E>
  struct letfn : expression_base
  {
    using pair_type = std::pair<obj::symbol_ptr, native_box<E>>;

    letfn(expression_position const type, native_bool const needs_box, local_frame_ptr const f)
      : expression_base{ gc{}, type, f, needs_box }
    {
    }

    native_vector<pair_type> pairs;
    do_<E> body;

    void propagate_position(expression_position const pos)
    {
      position = pos;
      body.propagate_position(pos);
    }

    object_ptr to_runtime_data() const
    {
      object_ptr pair_maps(make_box<obj::persistent_vector>());
      for(auto const &e : pairs)
      {
        pair_maps = conj(
          pair_maps,
          make_box<obj::persistent_vector>(
            std::in_place,
            jank::detail::to_runtime_data(frame->find_local_or_capture(e.first).unwrap().binding),
            e.second->to_runtime_data()));
      }

      return merge(static_cast<expression_base const *>(this)->to_runtime_data(),
                   obj::persistent_array_map::create_unique(make_box("__type"),
                                                            make_box("expr::letfn"),
                                                            make_box("pairs"),
                                                            pair_maps,
                                                            make_box("body"),
                                                            jank::detail::to_runtime_data(body)));
    }
  };
}
