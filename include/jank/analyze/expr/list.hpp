#pragma once

#include <vector>

#include <jank/runtime/obj/list.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct list
  {
    runtime::obj::list_ptr data;
    std::vector<E> exprs;
  };
}
