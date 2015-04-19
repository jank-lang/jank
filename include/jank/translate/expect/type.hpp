#pragma once

#include <stdexcept>
#include <cstdint>

#include <jank/parse/cell/cell.hpp>
#include <jank/translate/expect/error/type/type.hpp>

namespace jank
{
  namespace translate
  {
    namespace expect
    {
      template <parse::cell::type C, typename Cell>
      decltype(auto) type(Cell &&c)
      {
        auto const type(static_cast<parse::cell::type>(c.which()));
        if(type != C)
        {
          throw error::type::type<>
          {
            std::string{ "expected: " } +
            parse::cell::type_string<C>() +
            ", found: " +
            parse::cell::type_string(type)
          };
        }
        return boost::get<parse::cell::type_variant<C>>(c);
      }
    }
  }
}
