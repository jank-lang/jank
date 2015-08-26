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
        template <typename T>
        struct genericity
        {
          std::vector<parameter<T>> parameters;
        };

        template <typename T>
        bool operator ==(genericity<T> const &lhs, genericity<T> const &rhs)
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
        template <typename T>
        bool operator !=(genericity<T> const &lhs, genericity<T> const &rhs)
        { return !(lhs == rhs); }
      }
    }
  }
}
