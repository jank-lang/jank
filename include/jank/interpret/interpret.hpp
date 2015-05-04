#pragma once

#include <jank/parse/cell/cell.hpp>
#include <jank/interpret/environment/scope.hpp>

namespace jank
{
  namespace interpret
  {
    parse::cell::cell interpret(environment::scope &env, parse::cell::list &root);
  }
}
