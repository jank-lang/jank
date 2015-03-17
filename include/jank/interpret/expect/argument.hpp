#pragma once

#include <stdexcept>
#include <cstdint>

#include <jank/interpret/cell/cell.hpp>
#include <jank/interpret/expect/error/type/overload.hpp>

namespace jank
{
  namespace interpret
  {
    namespace expect
    {
      inline void args(cell::list const &c, std::size_t const count)
      {
        if(c.data.size() != count + 1) /* The first atom is not an arg. */
        {
          throw expect::error::type::overload
          {
            "invalid argument count (expected: " + std::to_string(count) + ", "
            "found: " + std::to_string(c.data.size() - 1) + ")"
          };
        }
      }

      inline void at_least_args(cell::list const &c, std::size_t const count)
      {
        if(c.data.size() < count + 1) /* The first atom is not an arg. */
        {
          throw expect::error::type::overload
          {
            "invalid argument count (expected at least: " + std::to_string(count) +
            ", found: " + std::to_string(c.data.size() - 1) + ")"
          };
        }
      }
    }
  }
}
