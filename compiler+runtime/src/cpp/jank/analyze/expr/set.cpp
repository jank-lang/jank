#include <jank/analyze/expr/set.hpp>
#include <jank/detail/to_runtime_data.hpp>

namespace jank::analyze::expr
{
  using namespace jank::runtime;

  set::set(expression_position const position,
           local_frame_ptr const frame,
           native_bool const needs_box,
           native_vector<expression_ptr> &&data_exprs,
           jtl::option<runtime::object_ptr> const &meta)
    : expression{ expr_kind, position, frame, needs_box }
    , data_exprs{ std::move(data_exprs) }
    , meta{ meta }
  {
  }

  object_ptr set::to_runtime_data() const
  {
    auto pair_maps(make_box<obj::persistent_vector>());
    for(auto const &e : data_exprs)
    {
      pair_maps = pair_maps->conj(e->to_runtime_data());
    }

    return merge(expression::to_runtime_data(),
                 obj::persistent_array_map::create_unique(make_box("data_exprs"),
                                                          pair_maps,
                                                          make_box("meta"),
                                                          jank::detail::to_runtime_data(meta)));
  }
}
