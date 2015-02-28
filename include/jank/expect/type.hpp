#pragma once

#include <stdexcept>
#include <cstdint>

#include <jank/cell/cell.hpp>
#include <jank/expect/error/type/type.hpp>

namespace jank
{
  namespace expect
  {
    template <cell::type C, typename Cell>
    decltype(auto) type(Cell &&c)
    {
      auto const type(static_cast<cell::type>(c.which()));
      if(type != C)
      {
        throw error::type::type<>
        { 
          std::string{ "expected " } +
          cell::type_string<C>() +
          ", found: " +
          cell::type_string(type)
        };
      }
      return boost::get<cell::type_variant_t<C>>(c);
    }
  }
}
