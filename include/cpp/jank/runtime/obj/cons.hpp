#pragma once

#include <jank/runtime/behavior/seqable.hpp>

namespace jank::runtime::obj
{
  struct cons : behavior::sequence
  {
    cons(object_ptr const head, behavior::sequence_ptr const tail);

    behavior::sequence_ptr seq() const override;
    object_ptr first() const override;
    behavior::sequence_ptr next() const override;
    behavior::sequence_ptr next_in_place() override;
    object_ptr next_in_place_first() override;

    void to_string(fmt::memory_buffer &buff) const override;
    native_string to_string() const override;
    native_integer to_hash() const override;

    object_ptr head{};
    behavior::sequence_ptr tail{};
    mutable size_t hash{};
  };
}
