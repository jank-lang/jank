#include <jank/analyze/expr/call.hpp>
#include <jank/detail/to_runtime_data.hpp>

namespace jank::analyze::expr
{
  using namespace jank::runtime;

  call::call(expression_position const position,
             local_frame_ptr const frame,
             bool const needs_box,
             expression_ref const source,
             native_vector<expression_ref> &&arg_exprs,
             runtime::obj::persistent_list_ref const form)
    : expression{ expr_kind, position, frame, needs_box }
    , source_expr{ source }
    , arg_exprs{ std::move(arg_exprs) }
    , form{ form }
  {
  }

  object_ref call::to_runtime_data() const
  {
    auto arg_expr_maps(make_box<obj::persistent_vector>());
    for(auto const &e : arg_exprs)
    {
      arg_expr_maps = arg_expr_maps->conj(e->to_runtime_data());
    }

    return merge(expression::to_runtime_data(),
                 obj::persistent_array_map::create_unique(make_box("source_expr"),
                                                          source_expr->to_runtime_data(),
                                                          make_box("form"),
                                                          form,
                                                          make_box("arg_exprs"),
                                                          arg_expr_maps));
  }

  void call::walk(std::function<void(jtl::ref<expression>)> const &f)
  {
    f(source_expr);
    for(auto const arg_expr : arg_exprs)
    {
      f(arg_expr);
    }
    expression::walk(f);
  }
}
