#pragma once

#include <jank/analyze/expression_base.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct do_ : expression_base
  { native_vector<native_box<E>> body; };
}
