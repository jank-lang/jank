#pragma once

#include <prelude/object.hpp>

namespace jank
{
  /* TODO: Laziness. */
  inline object mapv(object const &f, object const &seq)
  {
    /* TODO: What if f is a fn ptr, not an object? */
    return seq.visit
    (
      [&](auto &&data) -> object
      {
        using T = std::decay_t<decltype(data)>;
        /* TODO: Generic seq handling. */
        auto constexpr is_vector(std::is_same_v<T, detail::vector>);
        auto constexpr is_set(std::is_same_v<T, detail::set>);
        auto constexpr is_map(std::is_same_v<T, detail::map>);

        if constexpr(is_vector || is_set || is_map)
        {
          detail::vector ret;
          ret.reserve(data.size());

          if constexpr(is_vector || is_set)
          {
            for(auto const &e : data)
            { ret.push_back(detail::invoke(&f, e)); }
          }
          else if constexpr(is_map)
          {
            for(auto const &p : data)
            { ret.push_back(detail::invoke(&f, object{ detail::vector{ p.first, p.second } })); }
          }

          return object{ ret };
        }
        else
        {
          /* TODO: Throw an error. */
          std::cout << "not a seq" << std::endl;
          return JANK_NIL;
        }
      }
    );
  }
}
