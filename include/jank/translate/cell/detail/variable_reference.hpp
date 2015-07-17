#pragma once

#include <jank/translate/cell/detail/variable_definition.hpp>

namespace jank
{
  namespace translate
  {
    namespace cell
    {
      namespace detail
      {
        template <typename C>
        struct variable_reference
        {
          variable_definition<C> definition;
        };
      }
    }
  }
}
