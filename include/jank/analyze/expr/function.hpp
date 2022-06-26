#pragma once

#include <vector>
#include <list>

#include <jank/runtime/obj/symbol.hpp>
#include <jank/analyze/frame.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct function
  {
    std::vector<runtime::obj::symbol_ptr> params;
    std::list<E> body;
    frame<E> local_frame;
  };
}
