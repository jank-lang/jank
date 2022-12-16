#pragma once

#include <vector>

#include <jank/runtime/object.hpp>
#include <jank/analyze/expression_base.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct vector : expression_base
  { std::vector<std::shared_ptr<E>> data_exprs; };
}
