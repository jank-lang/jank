#pragma once

#include <jtl/trait/predicate.hpp>
#include <jtl/memory.hpp>

namespace jank::runtime
{
  template <typename Input, typename Output>
  struct convert;

  template <typename Input, typename Output>
  concept convertible = requires(Input const &t) {
    { convert<Input, Output>::call(t) } -> jtl::is_convertible<Output>;
  };

  /* Given any T, getting a T from it requires no conversion. */
  template <typename T>
  struct convert<T, T>
  {
    static constexpr T &call(T &t)
    {
      return t;
    }

    static constexpr T call(T const &t)
    {
      return t;
    }

    static constexpr T call(T &&t)
    {
      return jtl::move(t);
    }
  };
}
