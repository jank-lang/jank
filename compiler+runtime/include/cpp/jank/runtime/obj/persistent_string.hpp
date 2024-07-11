#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/obj/persistent_string_sequence.hpp>

namespace jank::runtime
{
  /* TODO: Seqable. */
  template <>
  struct static_object<object_type::persistent_string> : gc
  {
    static constexpr native_bool pointer_free{ false };

    static_object() = default;
    static_object(static_object &&) = default;
    static_object(static_object const &) = default;
    static_object(native_persistent_string const &d);
    static_object(native_persistent_string &&d);

    static native_box<static_object> empty()
    {
      static auto const ret(make_box<static_object>());
      return ret;
    }

    /* behavior::objectable */
    native_bool equal(object const &) const;
    native_persistent_string const &to_string() const;
    void to_string(fmt::memory_buffer &buff) const;
    native_hash to_hash() const;

    result<native_box<static_object>, native_persistent_string>
    substring(native_integer start) const;
    result<native_box<static_object>, native_persistent_string>
    substring(native_integer start, native_integer end) const;

    /* Returns -1 when not found. Turns the arg into a string, so it accepts anything.
     * Searches for the whole string, not just a char. */
    native_integer first_index_of(object_ptr const c) const;
    native_integer last_index_of(object_ptr const c) const;

    /* behavior::countable */
    size_t count() const;

    /* behavior::seqable */
    obj::persistent_string_sequence_ptr seq() const;
    obj::persistent_string_sequence_ptr fresh_seq() const;

    object base{ object_type::persistent_string };
    native_persistent_string data;
  };

  namespace obj
  {
    using persistent_string = static_object<object_type::persistent_string>;
    using persistent_string_ptr = native_box<persistent_string>;
  }
}
