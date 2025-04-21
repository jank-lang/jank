#include <jank/analyze/expr/do.hpp>
#include <jank/detail/to_runtime_data.hpp>
#include <jank/analyze/visit.hpp>

namespace jank::analyze::expr
{
  using namespace jank::runtime;

  do_::do_()
    : expression{ expr_kind }
  {
  }

  do_::do_(expression_position const position,
           local_frame_ptr const frame,
           bool const needs_box)
    : expression{ expr_kind, position, frame, needs_box }
  {
  }

  do_::do_(expression_position const position,
           local_frame_ptr const frame,
           bool const needs_box,
           native_vector<expression_ref> &&values)
    : expression{ expr_kind, position, frame, needs_box }
    , values{ std::move(values) }
  {
  }

  void do_::propagate_position(expression_position const pos)
  {
    position = pos;
    if(!values.empty())
    {
      values.back()->propagate_position(pos);
    }
  }

  object_ref do_::to_runtime_data() const
  {
    auto body_maps{ make_box<obj::persistent_vector>() };
    for(auto const &e : values)
    {
      visit_expr(
        [&](auto const typed_e) { body_maps = body_maps->conj(typed_e->to_runtime_data()); },
        e);
    }
    return merge(expression::to_runtime_data(),
                 obj::persistent_array_map::create_unique(make_box("body"), body_maps));
  }
}
