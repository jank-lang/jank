#include <jank/analyze/expr/cpp_new.hpp>
#include <jank/detail/to_runtime_data.hpp>

namespace jank::analyze::expr
{
  using namespace jank::runtime;

  cpp_new::cpp_new(expression_position const position,
                   local_frame_ptr const frame,
                   bool const needs_box,
                   jtl::ptr<void> const type,
                   expression_ref const value_expr)
    : expression{ expr_kind, position, frame, needs_box }
    , type{ type }
    , value_expr{ value_expr }
  {
  }

  void cpp_new::propagate_position(expression_position const pos)
  {
    position = pos;
  }

  object_ref cpp_new::to_runtime_data() const
  {
    /* TODO: Fill in. */
    return merge(expression::to_runtime_data(), obj::persistent_array_map::create_unique());
  }
}
