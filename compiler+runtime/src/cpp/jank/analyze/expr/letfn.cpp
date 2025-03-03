#include <jank/analyze/expr/letfn.hpp>
#include <jank/detail/to_runtime_data.hpp>
#include <jank/analyze/local_frame.hpp>

namespace jank::analyze::expr
{
  using namespace jank::runtime;

  letfn::letfn(expression_position const position,
               local_frame_ptr const frame,
               native_bool const needs_box,
               do_ptr const body)
    : expression{ expr_kind, position, frame, needs_box }
    , body{ body }
  {
  }

  void letfn::propagate_position(expression_position const pos)
  {
    position = pos;
    body->propagate_position(pos);
  }

  object_ptr letfn::to_runtime_data() const
  {
    auto pair_maps(make_box<obj::persistent_vector>());
    for(auto const &e : pairs)
    {
      pair_maps = pair_maps->conj(make_box<obj::persistent_vector>(
        std::in_place,
        jank::detail::to_runtime_data(*frame->find_local_or_capture(e.first).unwrap().binding),
        e.second->to_runtime_data()));
    }

    return merge(expression::to_runtime_data(),
                 obj::persistent_array_map::create_unique(make_box("pairs"),
                                                          pair_maps,
                                                          make_box("body"),
                                                          body->to_runtime_data()));
  }
}
