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
        template <typename C>
        struct type_definition
        {
          std::string name;
          translate::type::generic::genericity<type_definition> generics;
          /* TODO: vector<member> */
        };

        template <typename C>
        bool operator ==
        (type_definition<C> const &lhs, type_definition<C> const &rhs)
        {
          return
          (
            lhs.name == rhs.name &&
            lhs.generics == rhs.generics
          );
        }

        template <typename C>
        bool operator !=
        (type_definition<C> const &lhs, type_definition<C> const &rhs)
        {
          return
          (
            lhs.name != rhs.name ||
            lhs.generics != rhs.generics
          );
        }
      }
    }
  }
}
