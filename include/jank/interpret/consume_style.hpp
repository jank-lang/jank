#pragma once

#include <type_traits>

namespace jank
{
  namespace interpret
  {
    /* Says whether or not values from function calls should be returned.
     * By default, any value that isn't from a return statement is ignored.
     * This is used when interpreting a parameter to a function, since the value
     * will be used. Furthermore, in the repl, all values are wanted. */
    enum class consume_style
    {
      normal,
      greedy,
      all
    };

    inline bool operator >(consume_style const lhs, consume_style const rhs)
    {
      using underlying = std::underlying_type_t<consume_style>;
      return static_cast<underlying>(lhs) > static_cast<underlying>(rhs);
    }
  }
}
