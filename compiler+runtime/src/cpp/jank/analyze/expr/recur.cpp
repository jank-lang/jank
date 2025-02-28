#include <jank/analyze/expr/recur.hpp>
#include <jank/detail/to_runtime_data.hpp>

namespace jank::analyze::expr
{
  using namespace jank::runtime;

  recur::recur(expression_position const position,
               local_frame_ptr const frame,
               native_bool const needs_box,
               runtime::obj::persistent_list_ptr const args,
               native_vector<expression_ptr> &&arg_exprs)
    : expression{ expr_kind, position, frame, needs_box }
    , args{ args }
    , arg_exprs{ std::move(arg_exprs) }
  {
  }

  object_ptr recur::to_runtime_data() const
  {
    auto arg_expr_maps(make_box<obj::persistent_vector>());
    for(auto const &e : arg_exprs)
    {
      arg_expr_maps = arg_expr_maps->conj(e->to_runtime_data());
    }

    return merge(expression::to_runtime_data(),
                 obj::persistent_array_map::create_unique(make_box("args"),
                                                          args,
                                                          make_box("arg_exprs"),
                                                          arg_expr_maps));
  }
}
