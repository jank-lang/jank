#include <jank/analyze/expr/cpp_member_call.hpp>
#include <jank/detail/to_runtime_data.hpp>

namespace jank::analyze::expr
{
  using namespace jank::runtime;

  cpp_member_call::cpp_member_call(expression_position const position,
                                   local_frame_ptr const frame,
                                   bool const needs_box,
                                   jtl::ptr<void> const type,
                                   jtl::ptr<void> const fn,
                                   native_vector<expression_ref> &&arg_exprs)
    : expression{ expr_kind, position, frame, needs_box }
    , parent_type{ type }
    , fn{ fn }
    , arg_exprs{ jtl::move(arg_exprs) }
  {
  }

  void cpp_member_call::propagate_position(expression_position const pos)
  {
    position = pos;
  }

  object_ref cpp_member_call::to_runtime_data() const
  {
    /* TODO: Fill in. */
    return merge(expression::to_runtime_data(), obj::persistent_array_map::create_unique());
  }

  void cpp_member_call::walk(std::function<void(jtl::ref<expression>)> const &f)
  {
    for(auto const arg_expr : arg_exprs)
    {
      f(arg_expr);
    }
    expression::walk(f);
  }
}
