#pragma once

#include <jank/native_box.hpp>

namespace jank::runtime::behavior
{
  template <typename T>
  concept associatively_readable = requires(T * const t) {
    /* Returns the found value or nil. */
    { t->get(object_ptr{}) } -> std::convertible_to<object_ptr>;

    /* Returns the found value or the second arg. */
    { t->get(object_ptr{}, object_ptr{}) } -> std::convertible_to<object_ptr>;

    /* Returns a map entry (vector of key and value) or nil. */
    { t->get_entry(object_ptr{}) } -> std::convertible_to<object_ptr>;

    { t->contains(object_ptr{}) } -> std::convertible_to<native_bool>;
  };
}
