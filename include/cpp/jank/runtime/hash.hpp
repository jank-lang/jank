#pragma once

#include <functional>

#include <jank/runtime/detail/type.hpp>

/* TODO: Move file to detail dir */
namespace jank::runtime::detail
{
  /* Very much borrowed from boost. */
  template <typename T>
  detail::integer_type hash_combine(size_t const seed, T const &t)
  {
    static std::hash<T> hasher{};
    return static_cast<detail::integer_type>
    (seed ^ hasher(t) + 0x9e3779b9 + (seed << 6) + (seed >> 2));
  }
}
