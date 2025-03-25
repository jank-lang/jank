#include <jank/analyze/expr/recursion_reference.hpp>
#include <jank/detail/to_runtime_data.hpp>

namespace jank::analyze::expr
{
  using namespace jank::runtime;

  recursion_reference::recursion_reference(expression_position const position,
                                           local_frame_ptr const frame,
                                           native_bool const needs_box,
                                           function_context_ptr const fn_ctx)
    : expression{ expr_kind, position, frame, needs_box }
    , fn_ctx{ fn_ctx }
  {
  }

  object_ptr recursion_reference::to_runtime_data() const
  {
    return merge(
      expression::to_runtime_data(),
      runtime::obj::persistent_array_map::create_unique(make_box("fn_ctx"),
                                                        jank::detail::to_runtime_data(fn_ctx)));
  }
}
