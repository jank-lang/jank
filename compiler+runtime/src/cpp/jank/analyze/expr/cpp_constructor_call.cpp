#include <jank/analyze/expr/cpp_constructor_call.hpp>
#include <jank/detail/to_runtime_data.hpp>

namespace jank::analyze::expr
{
  using namespace jank::runtime;

  cpp_constructor_call::cpp_constructor_call()
    : expression{ expr_kind }
  {
  }

  cpp_constructor_call::cpp_constructor_call(expression_position const position,
                                             local_frame_ptr const frame,
                                             bool const needs_box,
                                             jtl::ptr<void> const type,
                                             jtl::ptr<void> const fn,
                                             native_vector<expression_ref> &&arg_exprs)
    : expression{ expr_kind, position, frame, needs_box }
    , type{ type }
    , fn{ fn }
    , arg_exprs{ jtl::move(arg_exprs) }
  {
  }

  void cpp_constructor_call::propagate_position(expression_position const pos)
  {
    position = pos;
  }

  object_ref cpp_constructor_call::to_runtime_data() const
  {
    /* TODO: Fill in. */
    return merge(expression::to_runtime_data(), obj::persistent_array_map::create_unique());
  }
}
