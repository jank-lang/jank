#pragma once

#include <jank/analyze/expression.hpp>

namespace jank::analyze::pass
{
  expression_ref optimize(expression_ref expr);
}
