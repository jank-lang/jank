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
      parse::cell::cell native_function_call
      (
        std::shared_ptr<environment::scope> const&,
        translate::cell::native_function_call const&
      );
    }
  }
}
