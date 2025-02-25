#pragma once

#include <jank/runtime/native_box.hpp>

namespace jank::runtime::behavior
{
  template <typename T>
  concept reduceable = requires(T * const t) {
    {
      t->reduce(std::function<object *(object *, object *)>{}, object_ptr{})
    } -> std::convertible_to<object_ptr>;
  };

  template <typename T>
  concept reduceable_kv = requires(T * const t) {
    {
      t->reduce_kv(std::function<object *(object *, object *, object *)>{}, object_ptr{})
    } -> std::convertible_to<object_ptr>;
  };
}
