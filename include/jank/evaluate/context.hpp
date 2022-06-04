#pragma once

#include <jank/runtime/object.hpp>
#include <jank/analyze/expression.hpp>

namespace jank::evaluate
{
  struct context
  {
    context() = default;
    context(context const &) = default;
    context(context &&) = default;

    runtime::object_ptr eval(analyze::expression const &);
  };
}
