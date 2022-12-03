#pragma once

#include <vector>

#include <jank/runtime/object.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct vector
  { std::vector<std::shared_ptr<E>> data_exprs; };
}
