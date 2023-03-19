#pragma once

#include <jank/runtime/behavior/seqable.hpp>

namespace jank::runtime::obj
{
  struct iterator : behavior::sequence
  {
    iterator(behavior::callable_ptr const fn, object_ptr const start);

    behavior::sequence_ptr seq() const override;
    object_ptr first() const override;
    behavior::sequence_ptr next() const override;
    behavior::sequence_ptr next_in_place() override;
    object_ptr next_in_place_first() override;

    void to_string(fmt::memory_buffer &buff) const override;
    native_string to_string() const override;

    /* TODO: Support chunking. */
    behavior::callable_ptr fn{};
    object_ptr current{};
    object_ptr previous{};
    mutable behavior::sequence_ptr cached_next{};
  };
}
