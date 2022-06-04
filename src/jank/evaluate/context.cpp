#include <iostream>

#include <jank/runtime/ns.hpp>
#include <jank/evaluate/context.hpp>

namespace jank::evaluate
{
  runtime::object_ptr context::eval(analyze::expression const &)
  {
    return {};
  }
}
