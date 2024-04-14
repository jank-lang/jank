#pragma once

#include <jank/native_box.hpp>

namespace jank::runtime::behavior
{
  template <typename T>
  concept associatively_writable = requires(T * const t) {
    {
      t->assoc(object_ptr{}, object_ptr{})
    } -> std::convertible_to<object_ptr>;

    {
      t->dissoc(object_ptr{})
    } -> std::convertible_to<object_ptr>;
  };

  template <typename T>
  concept associatively_writable_in_place = requires(T * const t) {
    {
      t->assoc_in_place(object_ptr{}, object_ptr{})
    } -> std::convertible_to<object_ptr>;

    {
      t->dissoc_in_place(object_ptr{})
    } -> std::convertible_to<object_ptr>;
  };
}
