#include <jank/analyze/expr/throw.hpp>
#include <jank/detail/to_runtime_data.hpp>

namespace jank::analyze::expr
{
  using namespace jank::runtime;

  throw_::throw_(expression_position const position,
                 local_frame_ptr const frame,
                 bool const needs_box,
                 expression_ref const value)
    : expression{ expr_kind, position, frame, needs_box }
    , value{ value }
  {
  }

  object_ref throw_::to_runtime_data() const
  {
    return merge(expression::to_runtime_data(),
                 obj::persistent_array_map::create_unique(make_box("value"),
                                                          jank::detail::to_runtime_data(*value)));
  }
}
