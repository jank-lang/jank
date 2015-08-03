#pragma once

#include <jank/parse/cell/cell.hpp>
#include <jank/translate/cell/cell.hpp>

namespace jank
{
  namespace translate
  {
    namespace environment
    {
      namespace special
      {
        cell::cell do_statement
        (
          parse::cell::list const &input,
          cell::function_body const &outer_body
        );
      }
    }
  }
}
