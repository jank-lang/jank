#pragma once

#include <jank/runtime/behavior/seqable.hpp>

namespace jank::runtime::obj
{
  struct range : behavior::sequence
  {
    static constexpr bool pointer_free{ false };

    range(object_ptr const end);
    range(object_ptr const start, object_ptr const end);
    range(object_ptr const start, object_ptr const end, object_ptr const step);

    behavior::sequence_ptr seq() const final;
    behavior::sequence_ptr fresh_seq() const final;
    object_ptr first() const final;
    behavior::sequence_ptr next() const final;
    behavior::sequence_ptr next_in_place() final;
    object_ptr next_in_place_first() final;

    void to_string(fmt::memory_buffer &buff) const final;
    native_string to_string() const final;

    /* TODO: Support chunking. */
    object_ptr start{};
    object_ptr end{};
    object_ptr step{};
    mutable behavior::sequence_ptr cached_next{};
  };
}
