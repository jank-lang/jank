#pragma once

#include <vector>
#include <list>

#include <jank/runtime/obj/symbol.hpp>
#include <jank/analyze/local_frame.hpp>
#include <jank/analyze/expr/do.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct function
  {
    std::vector<runtime::obj::symbol_ptr> params;
    do_<E> body;
    local_frame<E> frame;
  };
}
