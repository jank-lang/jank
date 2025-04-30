#include <jank/analyze/expr/cpp_value.hpp>
#include <jank/detail/to_runtime_data.hpp>

namespace jank::analyze::expr
{
  using namespace jank::runtime;

  cpp_value::cpp_value(expression_position const position,
                       local_frame_ptr const frame,
                       bool const needs_box,
                       jtl::ptr<void> const type,
                       jtl::ptr<void> const scope,
                       value_kind const val_kind)
    : expression{ expr_kind, position, frame, needs_box }
    , type{ type }
    , scope{ scope }
    , val_kind{ val_kind }
  {
  }

  void cpp_value::propagate_position(expression_position const pos)
  {
    position = pos;
  }

  object_ref cpp_value::to_runtime_data() const
  {
    /* TODO: Fill in. */
    return merge(expression::to_runtime_data(), obj::persistent_array_map::create_unique());
  }
}
