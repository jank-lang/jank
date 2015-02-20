#pragma once

#include <stdexcept>
#include <cstdint>

#include <jank/cell/cell.hpp>

namespace jank
{
  namespace expect
  {
    template <cell::type C>
    auto const& type(cell::cell const &c)
    {
      auto const type(static_cast<cell::type>(c.which()));
      if(type != C)
      {
        throw std::invalid_argument
        { 
          std::string{"invalid argument type (expected "} +
          cell::type_string<C>() +
          ", found: " +
          cell::type_string(type) + ")"
        };
      }
      return boost::get<cell::type_variant_t<C>>(c);
    }
  }
}
