#pragma once

#include <jank/translate/cell/cell.hpp>
#include <jank/interpret/cell/cell.hpp>
#include <jank/interpret/environment/scope.hpp>

namespace jank
{
  namespace interpret
  {
    namespace environment
    {
      cell::cell resolve_value
      (
        std::shared_ptr<environment::scope> const &scope,
        translate::cell::cell const &c
      );
    }
  }
}
