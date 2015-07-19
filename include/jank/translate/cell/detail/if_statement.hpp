#pragma once

#include <jank/translate/cell/detail/type_reference.hpp>
#include <jank/translate/cell/detail/function_body.hpp>

namespace jank
{
  namespace translate
  {
    namespace cell
    {
      namespace detail
      {
        template <typename C>
        struct if_statement
        {
          C condition;
          function_body<C> true_body, false_body;
        };
      }
    }
  }
}
