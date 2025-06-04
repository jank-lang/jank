#include <jank/analyze/expr/cpp_builtin_operator_call.hpp>
#include <jank/detail/to_runtime_data.hpp>

namespace jank::analyze::expr
{
  using namespace jank::runtime;

  cpp_builtin_operator_call::cpp_builtin_operator_call(expression_position const position,
                                                       local_frame_ptr const frame,
                                                       bool const needs_box,
                                                       int const op,
                                                       native_vector<expression_ref> &&arg_exprs,
                                                       jtl::ptr<void> const type)
    : expression{ expr_kind, position, frame, needs_box }
    , op{ op }
    , arg_exprs{ std::move(arg_exprs) }
    , type{ type }
  {
  }

  object_ref cpp_builtin_operator_call::to_runtime_data() const
  {
    auto arg_expr_maps(make_box<obj::persistent_vector>());
    for(auto const &e : arg_exprs)
    {
      arg_expr_maps = arg_expr_maps->conj(e->to_runtime_data());
    }

    return merge(expression::to_runtime_data(),
                 obj::persistent_array_map::create_unique(make_box("op"),
                                                          make_box(op),
                                                          make_box("arg_exprs"),
                                                          arg_expr_maps));
  }
}
