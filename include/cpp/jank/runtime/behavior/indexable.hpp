#pragma once

#include <cstdlib> // size_t

namespace jank::runtime
{
  using object_ptr = native_box<struct object>;

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
