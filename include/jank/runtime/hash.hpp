#pragma once

#include <functional>

namespace jank::runtime::detail
{
  /* Very much borrowed from boost. */
  template <typename T>
  size_t hash_combine(size_t const seed, T const &t)
  {
    static std::hash<T> hasher{};
    return seed ^ hasher(t) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  }
}
