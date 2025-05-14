#pragma once

namespace jank::runtime::behavior
{
  template <typename T>
  concept stackable = requires(T * const t) {
    /* Returns the most efficient "top" item. For lists, this is from
     * the front, but for vectors this is from the back.
     *
     * Peek should return nil if the collection is empty. */
    { t->peek() } -> std::convertible_to<object_ref>;

    /* Removes the most efficient "top" item. For lists, this is from
     * the front, but for vectors this is from the back.
     *
     * Pop should throw if the collection is empty. */
    { t->pop() } -> std::convertible_to<object_ref>;
  };
}
