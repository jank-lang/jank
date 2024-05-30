#pragma once

#include <memory>

#include <jank/runtime/obj/persistent_list.hpp>
#include <jank/analyze/expression_base.hpp>
#include <jank/runtime/seq.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct call : expression_base
  {
    /* Var, local, or callable. */
    native_box<E> source_expr{};
    runtime::obj::persistent_list_ptr args{};
    native_vector<native_box<E>> arg_exprs;
    /* Do we recur through calling our own fn name? */
    native_bool is_named_recur{};

    runtime::object_ptr to_runtime_data() const
    {
      runtime::object_ptr arg_expr_maps(make_box<runtime::obj::persistent_vector>());
      for(auto const &e : arg_exprs)
      {
        arg_expr_maps = runtime::conj(arg_expr_maps, e->to_runtime_data());
      }

      return runtime::merge(
        static_cast<expression_base const *>(this)->to_runtime_data(),
        runtime::obj::persistent_array_map::create_unique(make_box("__type"),
                                                          make_box("expr::call"),
                                                          make_box("source_expr"),
                                                          source_expr->to_runtime_data(),
                                                          make_box("args"),
                                                          args,
                                                          make_box("arg_exprs"),
                                                          arg_expr_maps));
    }
  };
}
