#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/behavior/seqable.hpp>

namespace jank::runtime::obj
{
  using cons_ptr = native_box<struct cons>;
  using iterator_ptr = native_box<struct iterator>;

  /* TODO: Rename to iterator_sequence. */
  struct iterator : gc
  {
    static constexpr object_type obj_type{ object_type::iterator };
    static constexpr native_bool pointer_free{ false };
    static constexpr native_bool is_sequential{ true };

    iterator() = default;
    iterator(iterator &&) noexcept = default;
    iterator(iterator const &) = default;
    iterator(object_ptr const fn, object_ptr const start);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    native_persistent_string to_string();
    void to_string(fmt::memory_buffer &buff);
    native_persistent_string to_code_string();
    native_hash to_hash() const;

    /* behavior::seqable */
    iterator_ptr seq();
    iterator_ptr fresh_seq() const;

    /* behavior::sequenceable */
    object_ptr first() const;
    iterator_ptr next() const;
    obj::cons_ptr conj(object_ptr head) const;

    /* behavior::sequenceable_in_place */
    iterator_ptr next_in_place();

    object base{ object_type::iterator };
    /* TODO: Support chunking. */
    object_ptr fn{};
    object_ptr current{};
    object_ptr previous{};
    mutable iterator_ptr cached_next{};
  };
}
