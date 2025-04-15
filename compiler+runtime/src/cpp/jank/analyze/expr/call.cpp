#include <jank/analyze/expr/call.hpp>
#include <jank/detail/to_runtime_data.hpp>

namespace jank::analyze::expr
{
  using namespace jank::runtime;

  call::call(expression_position const position,
             local_frame_ptr const frame,
             native_bool const needs_box,
             expression_ref const source,
             runtime::obj::persistent_list_ref const form,
             native_vector<expression_ref> &&arg_exprs)
    : expression{ expr_kind, position, frame, needs_box }
    , source_expr{ source }
    , form{ form }
    , arg_exprs{ std::move(arg_exprs) }
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
}
