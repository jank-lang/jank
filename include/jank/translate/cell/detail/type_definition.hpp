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
        /* TODO: Add scope members to all of these. */
        struct type_definition
        {
          std::string name;
          std::vector<struct variable_definition> members;
        };

        inline bool operator ==(type_definition const &lhs, type_definition const &rhs)
        { return lhs.name == rhs.name; }
      }
    }
  }
}
