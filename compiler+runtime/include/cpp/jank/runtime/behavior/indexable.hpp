#pragma once

namespace jank::runtime::behavior
{
  /* Indexable is meant to provide efficient item access in a
   * collection, given an index. Alas, Clojure implements it
   * for sequences in O(n) as well.
   *
   * In jank, the `runtime::nth` functions will handle the O(n)
   * use cases and this behavior is reserved for more efficient
   * usage. */
  template <typename T>
  concept indexable = requires(T * const t) {
    /* Given an index, return the item at that index or throw. */
    { t->nth(object_ptr{}) } -> std::convertible_to<object_ptr>;

    /* Given an index, return the item at that index or return the fallback. */
    { t->nth(object_ptr{}, object_ptr{}) } -> std::convertible_to<object_ptr>;
  };
}
