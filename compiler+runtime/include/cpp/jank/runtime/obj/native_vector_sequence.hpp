#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using cons_ptr = native_box<struct cons>;
  using native_vector_sequence_ptr = native_box<struct native_vector_sequence>;

  struct native_vector_sequence : gc
  {
    static constexpr object_type obj_type{ object_type::native_vector_sequence };
    static constexpr native_bool pointer_free{ false };
    static constexpr native_bool is_sequential{ true };

    native_vector_sequence() = default;
    native_vector_sequence(native_vector_sequence &&) noexcept = default;
    native_vector_sequence(native_vector_sequence const &) = default;
    native_vector_sequence(native_vector<object_ptr> const &data, size_t index);
    native_vector_sequence(native_vector<object_ptr> &&data);
    native_vector_sequence(native_vector<object_ptr> &&data, size_t index);

    /* behavior::object_like */
    native_bool equal(object const &o) const;
    void to_string(fmt::memory_buffer &buff) const;
    native_persistent_string to_string() const;
    native_persistent_string to_code_string() const;
    native_hash to_hash();

    /* behavior::seqable */
    native_vector_sequence_ptr seq();
    native_vector_sequence_ptr fresh_seq();

    /* behavior::countable */
    size_t count() const;

    /* behavior::sequence */
    object_ptr first() const;
    native_vector_sequence_ptr next() const;
    obj::cons_ptr conj(object_ptr head);

    /* behavior::sequenceable_in_place */
    native_vector_sequence_ptr next_in_place();

    object base{ object_type::native_vector_sequence };
    native_vector<object_ptr> data{};
    size_t index{};
  };
}
