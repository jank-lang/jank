#pragma once

#include <jank/runtime/behavior/seqable.hpp>

namespace jank::runtime::obj
{
  struct iterator : behavior::sequence
  {
    iterator(behavior::callable_ptr const fn, object_ptr const start);

    behavior::sequence_ptr seq() const final;
    behavior::sequence_ptr fresh_seq() const final;
    object_ptr first() const final;
    behavior::sequence_ptr next() const final;
    behavior::sequence_ptr next_in_place() final;
    object_ptr next_in_place_first() final;

    void to_string(fmt::memory_buffer &buff) const final;
    native_string to_string() const final;

    /* TODO: Support chunking. */
    behavior::callable_ptr fn{};
    object_ptr current{};
    object_ptr previous{};
    mutable behavior::sequence_ptr cached_next{};
  };
}
