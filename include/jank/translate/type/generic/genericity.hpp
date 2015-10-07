#pragma once

#include <jank/translate/type/generic/parameter.hpp>

namespace jank
{
  namespace translate
  {
    namespace type
    {
      namespace generic
      {
        /* Def = detail::type_definition */
        template <typename Def>
        struct genericity
        {
          std::vector<parameter<Def>> parameters;
        };

        template <typename Def>
        bool operator <(genericity<Def> const &lhs, genericity<Def> const &rhs)
        {
          return std::lexicographical_compare
          (
            lhs.parameters.begin(), lhs.parameters.end(),
            rhs.parameters.begin(), rhs.parameters.end()
          );
        }

        template <typename Def>
        bool operator ==(genericity<Def> const &lhs, genericity<Def> const &rhs)
        {
          return
          (
            lhs.parameters.size() == rhs.parameters.size() &&
            std::equal
            (
              lhs.parameters.begin(), lhs.parameters.end(),
              rhs.parameters.begin()
            )
          );
        }
        template <typename Def>
        bool operator !=(genericity<Def> const &lhs, genericity<Def> const &rhs)
        { return !(lhs == rhs); }
      }
    }
  }
}
