#pragma once

#include <cstdlib> // size_t

#include <jank/runtime/detail/type.hpp>

namespace jank::runtime
{
  using object_ptr = detail::box_type<struct object>;

  namespace behavior
  {
    struct indexable
    {
      virtual ~indexable() = default;

      virtual object_ptr nth(size_t const i) const = 0;
      virtual object_ptr nth(size_t const i, object_ptr fallback) const = 0;
    };
  }
}
