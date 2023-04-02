#pragma once

#include <jank/native_box.hpp>

namespace jank::runtime::behavior
{
  struct associatively_writable
  {
    virtual ~associatively_writable() = default;
    virtual object_ptr assoc(object_ptr key, object_ptr val) const = 0;
  };
  using associatively_writable_ptr = native_box<associatively_writable>;
}
