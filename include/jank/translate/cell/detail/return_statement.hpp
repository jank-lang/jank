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
        struct return_statement
        {
          C cell;
          type_reference expected_type;
        };
      }
    }
  }
}
