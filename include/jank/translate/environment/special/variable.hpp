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
        cell::cell variable
        (parse::cell::list const &input, cell::function_body const &body);

        namespace detail
        {
          namespace variable
          {
            cell::cell make
            (
              parse::cell::list const &input,
              cell::function_body const &body,
              cell::detail::constness c
            );
          }
        }
      }
    }
  }
}
