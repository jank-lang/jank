#pragma once

#include <jank/translate/cell/detail/type_definition.hpp>

namespace jank
{
  namespace translate
  {
    namespace cell
    {
      namespace detail
      {
        struct type_reference
        {
          type_definition const &definition;
        };

        inline bool operator ==(type_reference const &lhs, type_reference const &rhs)
        { return lhs.definition == rhs.definition; }
      }
    }
  }
}
