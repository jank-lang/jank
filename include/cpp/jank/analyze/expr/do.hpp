#pragma once

#include <vector>

#include <jank/analyze/expression_base.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct do_ : expression_base
  { std::vector<std::shared_ptr<E>> body; };
}
