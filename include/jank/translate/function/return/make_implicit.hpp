#pragma once

#include <experimental/optional>

#include <jank/translate/cell/cell.hpp>

namespace jank
{
  namespace translate
  {
    namespace function
    {
      namespace ret
      {
        /* null */
        cell::cell make_implicit(cell::function_body::type const &body);

        /* Builds an implicit return statement from the last function call. */
        std::experimental::optional<cell::cell>
        make_implicit_from_call(cell::function_body::type const &body);
      }
    }
  }
}
