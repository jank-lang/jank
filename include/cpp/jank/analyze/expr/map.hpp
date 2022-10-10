#pragma once

#include <vector>

#include <jank/runtime/object.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct map
  {
    std::vector<std::pair<E, E>> data_exprs;
  };
}
