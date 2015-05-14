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
        struct variable_definition
        {
          std::string name;
          type_reference const &type;
        };
      }
    }
  }
}
