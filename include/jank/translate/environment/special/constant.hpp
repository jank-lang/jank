#pragma once

#include <jank/parse/cell/cell.hpp>
#include <jank/translate/cell/cell.hpp>
#include <jank/translate/environment/special/variable.hpp>

namespace jank
{
  namespace translate
  {
    namespace environment
    {
      namespace special
      {
        cell::cell constant
        (parse::cell::list const &input, cell::function_body const &body);
      }
    }
  }
}
