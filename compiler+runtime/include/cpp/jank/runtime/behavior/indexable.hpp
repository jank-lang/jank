#pragma once

namespace jank::runtime::behavior
{
  template <typename T>
  concept indexable = requires(T * const t) {
    /* Given an index, return the item at that index or throw. */
    { t->nth(object_ref{}) } -> std::convertible_to<object_ref>;

    /* Given an index, return the item at that index or return the fallback. */
    { t->nth(object_ref{}, object_ref{}) } -> std::convertible_to<object_ref>;
  };
}
