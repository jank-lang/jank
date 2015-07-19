#pragma once

#include <jank/parse/cell/cell.hpp>
#include <jank/translate/cell/cell.hpp>
#include <jank/interpret/environment/scope.hpp>

namespace jank
{
  namespace interpret
  {
    namespace detail
    {
      parse::cell::cell variable_definition
      (
        std::shared_ptr<environment::scope> const&,
        translate::cell::variable_definition const&
      );
    }
  }
}
