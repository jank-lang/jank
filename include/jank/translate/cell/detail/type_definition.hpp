#pragma once

#include <string>
#include <vector>

#include <jank/translate/type/generic/genericity.hpp>

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
          translate::type::generic::genericity<type_definition> generics;
        };

        inline bool operator ==
        (type_definition const &lhs, type_definition const &rhs)
        { return lhs.name == rhs.name; }
        inline bool operator !=
        (type_definition const &lhs, type_definition const &rhs)
        { return lhs.name != rhs.name; }
      }
    }
  }
}
