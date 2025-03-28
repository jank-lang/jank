#include <jank/analyze/expr/if.hpp>
#include <jank/detail/to_runtime_data.hpp>

namespace jank::analyze::expr
{
  using namespace jank::runtime;

  if_::if_(expression_position const position,
           local_frame_ptr const frame,
           native_bool const needs_box,
           expression_ref const condition,
           expression_ref const then,
           jtl::option<expression_ref> const &else_)
    : expression{ expr_kind, position, frame, needs_box }
    , condition{ condition }
    , then{ then }
    , else_{ else_ }
  {
  }

  void if_::propagate_position(expression_position const pos)
  {
    position = pos;
    then->propagate_position(pos);
    if(else_.is_some())
    {
      else_.unwrap()->propagate_position(pos);
    }
  }

  object_ptr if_::to_runtime_data() const
  {
    return merge(expression::to_runtime_data(),
                 obj::persistent_array_map::create_unique(make_box("condition"),
                                                          jank::detail::to_runtime_data(*condition),
                                                          make_box("then"),
                                                          jank::detail::to_runtime_data(*then),
                                                          make_box("else"),
                                                          jank::detail::to_runtime_data(else_)));
  }
}
