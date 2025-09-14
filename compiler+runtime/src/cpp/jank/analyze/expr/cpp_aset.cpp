#include <jank/analyze/expr/cpp_aset.hpp>
#include <jank/detail/to_runtime_data.hpp>

namespace jank::analyze::expr
{
  using namespace jank::runtime;

  cpp_aset::cpp_aset(expression_position const position,
                     local_frame_ptr const frame,
                     bool const needs_box,
                     expression_ref const array_expr,
                     expression_ref const index_expr,
                     expression_ref const value_expr,
                     jtl::ptr<void> const element_type) 
    : expression{ expr_kind, position, frame, needs_box }
    , array_expr{ array_expr }
    , index_expr{ index_expr }
    , value_expr{ value_expr }
    , element_type{ element_type }
  {
  }

  void cpp_aset::propagate_position(expression_position const pos)
  {
    position = pos;
  }

  object_ref cpp_aset::to_runtime_data() const
  {
    return merge(expression::to_runtime_data(),
                 obj::persistent_array_map::create_unique(
                   make_box("array_expr"), array_expr->to_runtime_data(),
                   make_box("index_expr"), index_expr->to_runtime_data(),
                   make_box("value_expr"), value_expr->to_runtime_data()));
  }

  void cpp_aset::walk(std::function<void(jtl::ref<expression>)> const &f)
  {
    f(array_expr);
    f(index_expr);
    f(value_expr);
    expression::walk(f);
  }
}