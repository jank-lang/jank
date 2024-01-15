#pragma once

namespace jank::runtime::behavior
{
  template <typename T>
  concept consable = requires(T * const t) {
    /* Appends the specified object to the beginning of the current sequence. However, if
     * the current sequence is empty, it must create a cons onto nullptr. It's invalid to
     * have a cons onto an empty sequence. */
    {
      t->cons(object_ptr{})
    }; // -> consable
  };
}
