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
        cell::function_body::type add_implicit_returns
        (cell::function_body::type body);
      }
    }
  }
}
