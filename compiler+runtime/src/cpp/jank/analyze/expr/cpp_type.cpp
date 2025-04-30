#include <jank/analyze/expr/cpp_type.hpp>
#include <jank/detail/to_runtime_data.hpp>

namespace jank::analyze::expr
{
  using namespace jank::runtime;

  cpp_type::cpp_type()
    : expression{ expr_kind }
  {
  }

  cpp_type::cpp_type(expression_position const position,
                     local_frame_ptr const frame,
                     bool const needs_box,
                     jtl::ptr<void> const type)
    : expression{ expr_kind, position, frame, needs_box }
    , type{ type }
  {
  }

  void cpp_type::propagate_position(expression_position const pos)
  {
    position = pos;
  }

  object_ref cpp_type::to_runtime_data() const
  {
    /* TODO: Fill in. */
    return merge(expression::to_runtime_data(), obj::persistent_array_map::create_unique());
  }
}
