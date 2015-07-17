#pragma once

#include <string>
#include <vector>

namespace jank
{
  namespace translate
  {
    namespace cell
    {
      namespace detail
      {
        struct type_definition
        {
          std::string name;
          /* TODO: Add member support. */
          //std::vector<struct variable_definition> members;
        };

        inline bool operator ==(type_definition const &lhs, type_definition const &rhs)
        { return lhs.name == rhs.name; }
        inline bool operator !=(type_definition const &lhs, type_definition const &rhs)
        { return lhs.name != rhs.name; }
      }
    }
  }
}
