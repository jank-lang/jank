#pragma once

#include <optional>

namespace jank
{
  template <typename T>
  using option = std::optional<T>;

  template <typename T, typename Decayed = std::decay_t<T>>
  option<Decayed> some(T &&t)
  { return { std::forward<T>(t) }; }
  inline constexpr std::nullopt_t none = std::nullopt;
}
