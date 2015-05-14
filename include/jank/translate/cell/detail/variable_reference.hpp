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
        struct variable_reference
        {
          variable_definition const &definition;
        };
      }
    }
  }
}
