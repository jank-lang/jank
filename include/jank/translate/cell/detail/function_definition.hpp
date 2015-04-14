#pragma once

#include <jank/translate/cell/detail/body.hpp>

namespace jank
{
  namespace translate
  {
    namespace cell
    {
      namespace detail
      {
        template <typename C>
        struct function_definition
        {
          /* TODO: argument list. */
          body<C> body;
        };
      }
    }
  }
}
