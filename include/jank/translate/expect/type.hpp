#pragma once

#include <stdexcept>
#include <cstdint>

#include <jank/parse/cell/cell.hpp>
#include <jank/parse/cell/trait.hpp>
#include <jank/translate/expect/error/type/type.hpp>

namespace jank
{
  namespace translate
  {
    namespace expect
    {
      template <parse::cell::type C, typename Cell>
      bool is(Cell const &c)
      { return static_cast<parse::cell::type>(c.which()) == C; }

      template <parse::cell::type C, typename Cell>
      decltype(auto) type(Cell &&c)
      {
        auto const type(static_cast<parse::cell::type>(c.which()));
        if(type != C)
        {
          throw error::type::type<>
          {
            std::string{ "expected: " } +
            parse::cell::trait::enum_to_string<C>() +
            ", found: " +
            parse::cell::trait::enum_to_string(type)
          };
        }
        return boost::get<parse::cell::trait::enum_to_type<C>>(c);
      }
    }
  }
}
