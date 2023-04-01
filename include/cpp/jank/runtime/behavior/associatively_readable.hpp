#pragma once

#include <jank/native_box.hpp>

namespace jank::runtime::behavior
{
  struct associatively_readable
  {
    virtual ~associatively_readable() = default;
    virtual object_ptr get(object_ptr key) const = 0;
    virtual object_ptr get(object_ptr key, object_ptr fallback) const = 0;
  };
}
