#pragma once

#include <prelude/object.hpp>

namespace jank
{
  /* + */
  inline object _gen_plus_(object const &l, object const &r)
  {
    return l.visit_with
    (
      [&](auto const &l_data, auto const &r_data) -> object
      {
        using L = std::decay_t<decltype(l_data)>;
        using R = std::decay_t<decltype(r_data)>;

        /* TODO: Trait for is_number_v */
        if constexpr((std::is_same_v<L, integer> || std::is_same_v<L, real>)
                     && (std::is_same_v<R, integer> || std::is_same_v<R, real>))
        {
          return object{ l_data + r_data };
        }
        else
        {
          /* TODO: Throw an error. */
          std::cout << "not a number" << std::endl;
          return JANK_NIL;
        }
      },
      r
    );
  }
}
