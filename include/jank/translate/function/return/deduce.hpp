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
        /* Deduces a function's return type based on the functions
         * used within. */
        cell::function_body::type deduce(cell::function_body::type body);
      }
    }
  }
}
