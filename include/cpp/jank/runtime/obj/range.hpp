#pragma once

#include <jank/runtime/behavior/seqable.hpp>

namespace jank::runtime::obj
{
  struct range : behavior::sequence
  {
    range(object_ptr const end);
    range(object_ptr const start, object_ptr const end);
    range(object_ptr const start, object_ptr const end, object_ptr const step);

    behavior::sequence_ptr seq() const override;
    object_ptr first() const override;
    behavior::sequence_ptr next() const override;
    behavior::sequence_ptr next_in_place() override;
    object_ptr next_in_place_first() override;

    void to_string(fmt::memory_buffer &buff) const override;
    native_string to_string() const override;

    /* TODO: Support chunking. */
    object_ptr start{};
    object_ptr end{};
    object_ptr step{};
    behavior::sequence_ptr cached_next{};
  };
}
