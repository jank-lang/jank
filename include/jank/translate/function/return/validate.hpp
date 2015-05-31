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
        void validate(cell::function_body &body);
      }
    }
  }
}
