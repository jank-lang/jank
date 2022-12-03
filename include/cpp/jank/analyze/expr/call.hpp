#pragma once

#include <memory>

#include <jank/runtime/obj/function.hpp>
#include <jank/runtime/obj/list.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct call
  {
    /* Var, local, or callable. */
    std::shared_ptr<E> source;
    runtime::obj::list_ptr args;
    std::vector<std::shared_ptr<E>> arg_exprs;
    option<size_t> required_packed_args{};
  };
}
