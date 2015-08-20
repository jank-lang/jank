#pragma once

#include <jank/translate/cell/cell.hpp>
#include <jank/interpret/cell/cell.hpp>
#include <jank/interpret/environment/scope.hpp>

namespace jank
{
  namespace interpret
  {
    namespace detail
    {
      cell::cell return_statement
      (
        std::shared_ptr<environment::scope> const&,
        translate::cell::return_statement const&
      );
    }
  }
}
