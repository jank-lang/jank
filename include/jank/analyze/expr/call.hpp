#pragma once

#include <jank/runtime/obj/function.hpp>
#include <jank/runtime/obj/list.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct call
  {
    /* Var, local, or callable. */
    E source;
    runtime::obj::list_ptr args;
    std::vector<E> arg_exprs;
  };
}
