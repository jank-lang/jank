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
      parse::cell::cell binding_definition
      (
        std::shared_ptr<environment::scope> const&,
        translate::cell::binding_definition const&
      );
    }
  }
}
