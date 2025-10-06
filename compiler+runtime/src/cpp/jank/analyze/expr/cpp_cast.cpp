#include <jank/analyze/expr/cpp_cast.hpp>
#include <jank/detail/to_runtime_data.hpp>

namespace jank::analyze::expr
{
  using namespace jank::runtime;

  cpp_cast::cpp_cast(expression_position const position,
                     local_frame_ptr const frame,
                     bool const needs_box,
                     jtl::ptr<void> const type,
                     jtl::ptr<void> const conversion_type,
                     conversion_policy const policy,
                     expression_ref const value_expr)
    : expression{ expr_kind, position, frame, needs_box }
    , type{ type }
    , conversion_type{ conversion_type }
    , value_expr{ value_expr }
    , policy{ policy }
  {
  }

  void cpp_cast::propagate_position(expression_position const pos)
  {
    position = pos;
  }

  object_ref cpp_cast::to_runtime_data() const
  {
    /* TODO: Fill in. */
    return merge(expression::to_runtime_data(), obj::persistent_array_map::create_unique());
  }

  void cpp_cast::walk(std::function<void(jtl::ref<expression>)> const &f)
  {
    f(value_expr);
    expression::walk(f);
  }
}
