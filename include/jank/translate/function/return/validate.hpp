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
        /* Validates the all control paths return a value. */
        cell::function_body::type validate(cell::function_body::type body);
      }
    }
  }
}
