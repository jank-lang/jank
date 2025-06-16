#include <jank/analyze/expr/cpp_raw.hpp>
#include <jank/detail/to_runtime_data.hpp>

namespace jank::analyze::expr
{
  using namespace jank::runtime;

  cpp_raw::cpp_raw(expression_position const position,
                   local_frame_ptr const frame,
                   bool const needs_box,
                   jtl::immutable_string const &code)
    : expression{ expr_kind, position, frame, needs_box }
    , code{ code }
  {
  }

  void cpp_raw::propagate_position(expression_position const pos)
  {
    position = pos;
  }

  object_ref cpp_raw::to_runtime_data() const
  {
    /* TODO: Fill in. */
    return merge(expression::to_runtime_data(), obj::persistent_array_map::create_unique());
  }
}
