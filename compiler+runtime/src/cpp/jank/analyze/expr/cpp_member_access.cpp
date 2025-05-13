#include <jank/analyze/expr/cpp_member_access.hpp>
#include <jank/detail/to_runtime_data.hpp>

namespace jank::analyze::expr
{
  using namespace jank::runtime;

  cpp_member_access::cpp_member_access(expression_position const position,
                                       local_frame_ptr const frame,
                                       bool const needs_box,
                                       jtl::ptr<void> const type,
                                       jtl::ptr<void> const scope,
                                       jtl::immutable_string const &name,
                                       expression_ref const obj_expr)
    : expression{ expr_kind, position, frame, needs_box }
    , type{ type }
    , scope{ scope }
    , name{ name }
    , obj_expr{ obj_expr }
  {
  }

  void cpp_member_access::propagate_position(expression_position const pos)
  {
    position = pos;
  }

  object_ref cpp_member_access::to_runtime_data() const
  {
    /* TODO: Fill in. */
    return merge(expression::to_runtime_data(), obj::persistent_array_map::create_unique());
  }
}
