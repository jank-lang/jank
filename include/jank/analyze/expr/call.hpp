#pragma once

#include <jank/runtime/obj/function.hpp>
#include <jank/runtime/obj/list.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct call
  {
    runtime::obj::function_ptr fn;
    runtime::obj::list_ptr args;
    std::vector<E> arg_exprs;
  };
}
