#pragma once

#include <jank/translate/cell/detail/function_definition.hpp>

namespace jank
{
  namespace translate
  {
    namespace cell
    {
      namespace detail
      {
        template <typename C>
        struct function_reference
        {
          function_definition<C> definition;
        };
      }
    }
  }
}
