#pragma once

#include <vector>

#include <jank/runtime/object.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct map
  { std::vector<std::pair<std::shared_ptr<E>, std::shared_ptr<E>>> data_exprs; };
}
