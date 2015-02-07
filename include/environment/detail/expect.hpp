#pragma once

#include <stdexcept>
#include <cstdint>

#include "cell.hpp"

namespace detail
{
  inline void expect_args(cell_list const &c, std::size_t const count)
  {
    if(c.data.size() != count + 1) /* The first atom is not an arg. */
    {
      throw std::invalid_argument
      {
        "invalid argument count (expected: " + std::to_string(count) + ", "
        "found: " + std::to_string(c.data.size() - 1) + ")"
      };
    }
  }

  inline void expect_at_least_args(cell_list const &c, std::size_t const count)
  {
    if(c.data.size() < count + 1) /* The first atom is not an arg. */
    {
      throw std::invalid_argument
      {
        "invalid argument count (expected at least: " + std::to_string(count) +
        ", found: " + std::to_string(c.data.size() - 1) + ")"
      };
    }
  }

  template <cell_type C>
  auto const& expect_type(cell const &c)
  {
    auto type(static_cast<cell_type>(c.which()));
    if(type != C)
    {
      throw std::invalid_argument
      { 
        std::string{"invalid argument type (expected "} + cell_type_string<C>() +
        ", found: " + cell_type_string(type) + ")"
      };
    }
    return boost::get<cell_type_variant_t<C>>(c);
  }
}
