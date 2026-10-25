#include <jank/analyze/expr/map.hpp>
#include <jank/detail/to_runtime_data.hpp>

namespace jank::analyze::expr
{
  using namespace jank::runtime;

  map::map(expression_position const position,
           local_frame_ptr const frame,
           bool const needs_box,
           object_ref const form,
           native_vector<std::pair<expression_ref, expression_ref>> &&data_exprs,
           jtl::option<expression_ref> const &meta)
    : expression{ expr_kind, position, frame, needs_box, form }
    , data_exprs{ std::move(data_exprs) }
    , meta{ meta }
  {
  }

  object_ref map::to_runtime_data() const
  {
    auto pair_maps(make_box<obj::persistent_vector>());
    for(auto const &e : data_exprs)
    {
      pair_maps = pair_maps->conj(make_box<obj::persistent_vector>(std::in_place,
                                                                   e.first->to_runtime_data(),
                                                                   e.second->to_runtime_data()));
    }

    return merge(expression::to_runtime_data(),
                 obj::persistent_array_map::create_unique(
                   make_box("data_exprs"),
                   pair_maps,
                   make_box("meta"),
                   meta.is_some() ? meta.unwrap()->to_runtime_data() : jank_nil));
  }

  void map::walk(std::function<void(jtl::ref<expression>)> const &f)
  {
    for(auto const &p : data_exprs)
    {
      f(p.first);
      f(p.second);
    }
    if(meta.is_some())
    {
      f(meta.unwrap());
    }
    expression::walk(f);
  }
}
