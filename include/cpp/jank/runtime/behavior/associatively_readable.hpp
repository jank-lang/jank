#pragma once

#include <jank/native_box.hpp>

namespace jank::runtime::behavior
{
  template <typename T>
  concept associatively_readable = requires(T * const t)
  {
    { t->get(object_ptr{}) } -> std::convertible_to<object_ptr>;
    { t->get(object_ptr{}, object_ptr{}) } -> std::convertible_to<object_ptr>;
  };
}
