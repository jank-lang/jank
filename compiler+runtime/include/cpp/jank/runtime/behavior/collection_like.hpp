#pragma once

namespace jank::runtime::behavior
{
  /* Collections are types which are seqable, conjable, and countable and which have
   * a way to get an empty object.
   *
   * This roughly follows Clojure's IPersistentCollection.
   */
  template <typename T>
  concept collection_like = requires(T * const t) {
    /* Returns an empty collection of the same type. */
    { T::empty() } -> std::convertible_to<object_ptr>;
  } && seqable<T> && conjable<T>;
}
