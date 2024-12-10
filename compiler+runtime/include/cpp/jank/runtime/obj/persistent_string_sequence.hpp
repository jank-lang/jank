#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/obj/cons.hpp>

namespace jank::runtime
{
  namespace obj
  {
    using persistent_string = static_object<object_type::persistent_string>;
    using persistent_string_ptr = native_box<persistent_string>;
  }

  template <>
  struct static_object<object_type::persistent_string_sequence> : gc
  {
    static constexpr native_bool pointer_free{ false };
    static constexpr native_bool is_sequential{ true };

    static_object() = default;
    static_object(static_object &&) = default;
    static_object(static_object const &) = default;
    static_object(obj::persistent_string_ptr const s);
    static_object(obj::persistent_string_ptr const s, size_t const i);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    void to_string(fmt::memory_buffer &buff) const;
    native_persistent_string to_string() const;
    native_persistent_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::countable */
    size_t count() const;

    /* behavior::seqable */
    native_box<static_object> seq();
    native_box<static_object> fresh_seq() const;

    /* behavior::sequenceable */
    object_ptr first() const;
    native_box<static_object> next() const;
    obj::cons_ptr conj(object_ptr head);

    /* behavior::sequenceable_in_place */
    native_box<static_object> next_in_place();

    object base{ object_type::persistent_string_sequence };
    obj::persistent_string_ptr str{};
    size_t index{};
  };

  namespace obj
  {
    using persistent_string_sequence = static_object<object_type::persistent_string_sequence>;
    using persistent_string_sequence_ptr = native_box<persistent_string_sequence>;
  }
}
