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
        /* Used both for explicit binding definitions and parameter definitions.
         * In the former case, the cell will contain the actual value supplied.
         * In the latter case, the cell isn't used; it's not known until run time. */
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
