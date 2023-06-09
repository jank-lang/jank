#pragma once

#include <jank/runtime/behavior/seqable.hpp>

namespace jank::runtime::obj
{
  struct cons : behavior::sequence
  {
    static constexpr bool pointer_free{ false };

    cons(object_ptr const head, behavior::sequence_ptr const tail);

    behavior::sequence_ptr seq() const final;
    behavior::sequence_ptr fresh_seq() const final;
    object_ptr first() const final;
    behavior::sequence_ptr next() const final;
    behavior::sequence_ptr next_in_place() final;
    object_ptr next_in_place_first() final;

    void to_string(fmt::memory_buffer &buff) const final;
    native_string to_string() const final;
    native_integer to_hash() const final;

    object_ptr head{};
    behavior::sequence_ptr tail{};
    mutable size_t hash{};
  };
}
