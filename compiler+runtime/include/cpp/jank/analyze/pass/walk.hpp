#pragma once

#include <jank/analyze/expression.hpp>

namespace jank::analyze::pass
{
  void postwalk(expression_ref const expr, std::function<void(expression_ref)> const &f);
  void prewalk(expression_ref const expr, std::function<void(expression_ref)> const &f);
}
