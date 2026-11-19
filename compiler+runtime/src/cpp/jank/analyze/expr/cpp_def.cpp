#include <jank/analyze/expr/cpp_def.hpp>
#include <jank/detail/to_runtime_data.hpp>

namespace jank::analyze::expr
{
  using namespace jank::runtime;

  cpp_def::cpp_def(expression_position const position,
                   local_frame_ptr const frame,
                   bool const needs_box,
                   jtl::ptr<void> const type,
                   runtime::obj::symbol_ref const name,
                   jtl::option<expression_ref> &value_expr)
    : expression{ expr_kind, position, frame, needs_box }
    , type{ type }
    , name{ name }
    , value_expr{ value_expr }
  {
  }

  void cpp_def::propagate_position(expression_position const pos)
  {
    position = pos;
  }

  object_ref cpp_def::to_runtime_data() const
  {
    /* TODO: Fill in. */
    return merge(expression::to_runtime_data(), obj::persistent_array_map::create_unique());
  }

  void cpp_def::walk(std::function<void(jtl::ref<expression>)> const &f)
  {
    if(value_expr.is_some())
    {
      f(value_expr.unwrap());
    }
    expression::walk(f);
  }
}
