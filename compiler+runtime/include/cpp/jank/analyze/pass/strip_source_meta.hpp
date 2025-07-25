#pragma once

#include <jank/analyze/expression.hpp>

namespace jank::analyze::pass
{
  expression_ref strip_source_meta(expression_ref expr);
}
