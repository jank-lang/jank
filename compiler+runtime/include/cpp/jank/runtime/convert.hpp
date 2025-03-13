#pragma once

#include <concepts>
#include <utility>

namespace jank::runtime
{
  template <typename Input, typename Output>
  struct convert;

  template <typename Input, typename Output>
  concept convertible = requires(Input const &t) {
    { convert<Input, Output>::call(t) } -> std::same_as<Output>;
  };

  /* Given any T, getting a T from it requires no conversion. */
  template <typename T>
  struct convert<T, T>
  {
    static T &call(T &t)
    {
      return t;
    }

    static T call(T const &t)
    {
      return t;
    }

    static T call(T &&t)
    {
      return std::move(t);
    }
  };
}
