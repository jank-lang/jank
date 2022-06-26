#pragma once

#include <vector>

#include <jank/runtime/object.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct vector
  {
    std::vector<E> data_exprs;
  };
}
