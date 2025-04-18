#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::behavior
{
  template <typename T>
  concept associatively_readable = requires(T const * const t) {
    /* Returns the found value or nil. */
    { t->get(object_ref{}) } -> std::convertible_to<object_ref>;

    /* Returns the found value or the second arg. */
    { t->get(object_ref{}, object_ref{}) } -> std::convertible_to<object_ref>;

    /* Returns a map entry (vector of key and value) or nil. */
    { t->get_entry(object_ref{}) } -> std::convertible_to<object_ref>;

    { t->contains(object_ref{}) } -> std::convertible_to<bool>;
  };
}
