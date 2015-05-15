#pragma once

#include <stdexcept>
#include <cstdint>

#include <jank/translate/cell/trait.hpp>
#include <jank/translate/expect/error/type/type.hpp>

namespace jank
{
  namespace translate
  {
    namespace expect
    {
      template <cell::type C, typename Cell>
      bool is(Cell const &c)
      { return static_cast<cell::type>(c.which()) == C; }

      template <cell::type C, typename Cell>
      decltype(auto) type(Cell &&c)
      {
        auto const type(static_cast<cell::type>(c.which()));
        if(type != C)
        {
          throw error::type::type<>
          {
            std::string{ "expected: " } +
            cell::trait::to_string<C>() +
            ", found: " +
            cell::trait::to_string(type)
          };
        }
        return boost::get<cell::trait::to_type<C>>(c);
      }
    }
  }
}
