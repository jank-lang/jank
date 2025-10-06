#include <jank/analyze/expr/cpp_call.hpp>
#include <jank/detail/to_runtime_data.hpp>

namespace Cpp
{
  std::string GetName(void *);
}

namespace jank::analyze::expr
{
  using namespace jank::runtime;

  cpp_call::cpp_call(expression_position const position,
                     local_frame_ptr const frame,
                     bool const needs_box,
                     jtl::ptr<void> const type,
                     expression_ref const source_expr,
                     native_vector<expression_ref> &&arg_exprs)
    : expression{ expr_kind, position, frame, needs_box }
    , type{ type }
    , source_expr{ source_expr }
    , arg_exprs{ jtl::move(arg_exprs) }
  {
  }

  cpp_call::cpp_call(expression_position const position,
                     local_frame_ptr const frame,
                     bool const needs_box,
                     jtl::ptr<void> const type,
                     expression_ref const source_expr,
                     native_vector<expression_ref> &&arg_exprs,
                     jtl::immutable_string const &function_code)
    : expression{ expr_kind, position, frame, needs_box }
    , type{ type }
    , source_expr{ source_expr }
    , arg_exprs{ jtl::move(arg_exprs) }
    , function_code{ function_code }
  {
  }

  void cpp_call::propagate_position(expression_position const pos)
  {
    position = pos;
  }

  object_ref cpp_call::to_runtime_data() const
  {
    auto arg_expr_maps(make_box<obj::persistent_vector>());
    for(auto const &e : arg_exprs)
    {
      arg_expr_maps = arg_expr_maps->conj(e->to_runtime_data());
    }

    return merge(expression::to_runtime_data(),
                 obj::persistent_array_map::create_unique(make_box("source"),
                                                          source_expr->to_runtime_data(),
                                                          make_box("arg_exprs"),
                                                          arg_expr_maps));
  }

  void cpp_call::walk(std::function<void(jtl::ref<expression>)> const &f)
  {
    for(auto const arg_expr : arg_exprs)
    {
      f(arg_expr);
    }
    expression::walk(f);
  }
}
