#pragma once

#include <memory>

#include <jank/runtime/obj/list.hpp>
#include <jank/analyze/expression_base.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct call : expression_base
  {
    /* Var, local, or callable. */
    native_box<E> source_expr{};
    runtime::obj::list_ptr args{};
    native_vector<native_box<E>> arg_exprs;
  };
}
