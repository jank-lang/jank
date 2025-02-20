#pragma once

namespace jank::runtime::behavior
{
  template <typename T>
  concept derefable = requires(T * const t) {
    { t->deref() } -> std::convertible_to<object_ptr>;
  };

  template <typename T>
  concept blocking_derefable = requires(T * const t) {
    { t->blocking_deref(object_ptr{}, object_ptr{}) } -> std::convertible_to<object_ptr>;
  };
}
