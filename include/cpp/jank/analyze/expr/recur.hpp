#pragma once

#include <memory>
#include <vector>

#include <jank/runtime/obj/list.hpp>
#include <jank/analyze/expression_base.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct recur : expression_base
  {
    runtime::obj::list_ptr args;
    std::vector<std::shared_ptr<E>> arg_exprs;
  };
}
