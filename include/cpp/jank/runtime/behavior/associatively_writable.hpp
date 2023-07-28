#pragma once

#include <jank/native_box.hpp>

namespace jank::runtime::behavior
{
  template <typename T>
  concept associatively_writable = requires(T * const t)
  {
    { t->assoc(object_ptr{}, object_ptr{}) } -> std::convertible_to<object_ptr>;
  };
}
