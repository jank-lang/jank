#include <jank/analyze/expr/case.hpp>
#include <jank/detail/to_runtime_data.hpp>
#include <jank/runtime/obj/persistent_hash_map.hpp>

namespace jank::analyze::expr
{
  using namespace jank::runtime;

  case_::case_(expression_position const position,
               local_frame_ptr const frame,
               native_bool const needs_box,
               expression_ptr const value_expr,
               native_integer const shift,
               native_integer const mask,
               expression_ptr const default_expr,
               native_vector<native_integer> &&keys,
               native_vector<expression_ptr> &&exprs)
    : expression{ expr_kind, position, frame, needs_box }
    , value_expr{ value_expr }
    , shift{ shift }
    , mask{ mask }
    , default_expr{ default_expr }
    , keys{ std::move(keys) }
    , exprs{ std::move(exprs) }
  {
  }

  void case_::propagate_position(expression_position const pos)
  {
    default_expr->propagate_position(pos);
    for(auto const expr : exprs)
    {
      expr->propagate_position(pos);
    }
    position = pos;
  }

  object_ptr case_::to_runtime_data() const
  {
    auto pairs{ make_box<obj::persistent_vector>() };
    for(size_t i{}; i < keys.size(); ++i)
    {
      pairs = pairs->conj(make_box<obj::persistent_vector>(std::in_place,
                                                           make_box(keys[i]),
                                                           exprs[i]->to_runtime_data()));
    }

    return merge(expression::to_runtime_data(),
                 obj::persistent_hash_map::create_unique(
                   std::make_pair(make_box("value_expr"), value_expr->to_runtime_data()),
                   std::make_pair(make_box("pairs"), pairs),
                   std::make_pair(make_box(shift), make_box("shift")),
                   std::make_pair(make_box("mask"), make_box(mask)),
                   std::make_pair(make_box("default_expr"), default_expr->to_runtime_data())));
  }
}
