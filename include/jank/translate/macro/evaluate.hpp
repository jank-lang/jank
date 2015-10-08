#pragma once

#include <jank/translate/cell/cell.hpp>
#include <jank/translate/environment/scope.hpp>

namespace jank
{
  namespace translate
  {
    namespace macro
    {
      /* For function calls, this does nothing. */
      template <typename T>
      T evaluate
      (
        T &&call,
        std::shared_ptr<environment::scope> const&
      )
      { return std::forward<T>(call); }

      /* For macros, this interprets in place and stores the results. */
      cell::macro_call evaluate
      (
        cell::macro_call &&call,
        std::shared_ptr<environment::scope> const &scope
      );
    }
  }
}
