#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/obj/cons.hpp>

namespace jank::runtime::obj
{
  using persistent_vector_ptr = native_box<struct persistent_vector>;
  using persistent_vector_sequence_ptr = native_box<struct persistent_vector_sequence>;

  struct persistent_vector_sequence : gc
  {
    static constexpr object_type obj_type{ object_type::persistent_vector_sequence };
    static constexpr native_bool pointer_free{ false };
    static constexpr native_bool is_sequential{ true };

    persistent_vector_sequence() = default;
    persistent_vector_sequence(persistent_vector_sequence &&) noexcept = default;
    persistent_vector_sequence(persistent_vector_sequence const &) = default;
    persistent_vector_sequence(obj::persistent_vector_ptr v);
    persistent_vector_sequence(obj::persistent_vector_ptr v, size_t i);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    void to_string(fmt::memory_buffer &buff) const;
    native_persistent_string to_string() const;
    native_persistent_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::countable */
    size_t count() const;

    /* behavior::seqable */
    persistent_vector_sequence_ptr seq();
    persistent_vector_sequence_ptr fresh_seq() const;

    /* behavior::sequenceable */
    object_ptr first() const;
    persistent_vector_sequence_ptr next() const;
    obj::cons_ptr conj(object_ptr head);

    /* behavior::sequenceable_in_place */
    persistent_vector_sequence_ptr next_in_place();

    object base{ object_type::persistent_vector_sequence };
    obj::persistent_vector_ptr vec{};
    size_t index{};
  };
}
