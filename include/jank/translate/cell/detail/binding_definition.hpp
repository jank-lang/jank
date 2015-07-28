#pragma once

#include <jank/translate/cell/detail/type_reference.hpp>

namespace jank
{
  namespace translate
  {
    namespace cell
    {
      namespace detail
      {
        template <typename C>
        struct binding_definition
        {
          std::string name;
          type_reference type;
          C cell;
        };
      }
    }
  }
}
