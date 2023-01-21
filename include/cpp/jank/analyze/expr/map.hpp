#pragma once

#include <jank/analyze/expression_base.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct map : expression_base
  { native_vector<std::pair<native_box<E>, native_box<E>>> data_exprs; };
}
