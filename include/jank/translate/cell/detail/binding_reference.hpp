#pragma once

#include <jank/translate/cell/detail/binding_definition.hpp>

namespace jank
{
  namespace translate
  {
    namespace cell
    {
      namespace detail
      {
        template <typename C>
        struct binding_reference
        {
          binding_definition<C> definition;
        };
      }
    }
  }
}
