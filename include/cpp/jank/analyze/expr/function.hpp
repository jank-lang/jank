#pragma once

#include <vector>
#include <list>

#include <jank/runtime/obj/symbol.hpp>
#include <jank/analyze/local_frame.hpp>
#include <jank/analyze/expr/do.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct function_arity
  {
    std::vector<runtime::obj::symbol_ptr> params;
    do_<E> body;
    local_frame_ptr frame;
  };

  template <typename E>
  struct function
  {
    option<std::string> name;
    std::vector<function_arity<E>> arities;
  };
}
