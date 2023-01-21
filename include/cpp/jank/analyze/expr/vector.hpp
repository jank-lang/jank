#pragma once

#include <jank/analyze/expression_base.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct vector : expression_base
  { native_vector<native_box<E>> data_exprs; };
}
