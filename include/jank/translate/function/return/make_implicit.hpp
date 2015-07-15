#pragma once

#include <vector>
#include <memory>

#include <jank/translate/cell/cell.hpp>

namespace jank
{
  namespace translate
  {
    namespace function
    {
      namespace ret
      {
        cell::return_statement make_implicit(); /* null */

        /* Builds an implicit return statement from the last function call. */
        std::experimental::optional<cell::return_statement>
        make_implicit(cell::function_body &body);
      }
    }
  }
}
