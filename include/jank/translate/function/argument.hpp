#pragma once

#include <string>
#include <vector>

#include <jank/translate/cell/type.hpp>

namespace jank
{
  namespace translate
  {
    namespace function
    {
      struct argument
      {
        std::string name;
        cell::type type;
      };
      using argument_list = std::vector<argument>;

      inline bool operator ==(argument_list const &lhs, argument_list const &rhs)
      {
        return (lhs.size() == rhs.size()) &&
                std::equal(lhs.begin(), lhs.end(), rhs.begin(),
                           [](auto const &lhs, auto const &rhs)
                           { return lhs.type == rhs.type; });
      }
    }
  }
}
