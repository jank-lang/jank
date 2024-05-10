#pragma once

namespace jank::runtime::behavior
{
  template <typename T>
  concept conjable = requires(T * const t) {
    /* Appends the specified object to the beginning of the current sequence. However, if
     * the current sequence is empty, it must create a cons onto nullptr. It's invalid to
     * have a cons onto an empty sequence. */
    {
      t->conj(object_ptr{})
    }; // -> consable
  };

  template <typename T>
  concept conjable_in_place = requires(T * const t) {
    {
      t->cons_in_place(object_ptr{})
    }; // -> consable_in_place
  };
}
