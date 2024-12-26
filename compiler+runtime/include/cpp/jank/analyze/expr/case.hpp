#pragma once

#include <jank/detail/to_runtime_data.hpp>
#include <jank/analyze/expression_base.hpp>
#include <jank/runtime/obj/persistent_sorted_map.hpp>
#include <jank/runtime/obj/persistent_hash_set.hpp>

namespace jank::analyze::expr
{
  using namespace jank::runtime;

  template <typename E>
  struct case_ : expression_base
  {
    enum switch_type : std::uint8_t
    {
      integers,
      hashes,
      hash_equiv,
      hash_identity,
    };

    native_box<E> value_expr{};
    native_integer shift{};
    native_integer mask{};
    native_box<E> default_expr{};
    std::vector<native_integer> transformed_keys{};
    std::vector<native_box<E>> exprs{};
    native_bool is_compact{};
    switch_type switch_type{};
    obj::persistent_hash_set_ptr collided_keys{};

    void propagate_position(expression_position const pos)
    {
      default_expr->propagate_position(pos);
      for(auto &expr : exprs)
      {
        expr->propagate_position(pos);
      }
      position = pos;
    }

    object_ptr to_runtime_data() const
    {
      return merge(static_cast<expression_base const *>(this)->to_runtime_data(),
                   obj::persistent_array_map::create_unique(make_box("__type"),
                                                            make_box("expr::case"),
                                                            make_box("value_expr"),
                                                            value_expr->to_runtime_data(),
                                                            make_box("shift"),
                                                            make_box(shift),
                                                            make_box("mask"),
                                                            make_box(mask),
                                                            make_box("default_expr"),
                                                            default_expr->to_runtime_data(),
                                                            // make_box("keys"),
                                                            // keys,
                                                            // make_box("body_exprs"),
                                                            // exprs,
                                                            make_box("is_compact"),
                                                            make_box(is_compact),
                                                            make_box("switch_type"),
                                                            make_box(switch_type),
                                                            make_box("collided_keys"),
                                                            collided_keys));
    }
  };
}
