#pragma once

namespace jank::runtime
{
  template <>
  struct static_object<object_type::native_vector_sequence> : gc
  {
    static constexpr native_bool pointer_free{ false };

    static_object() = default;
    static_object(static_object &&) = default;
    static_object(static_object const &) = default;
    static_object(native_vector<object_ptr> const &data, size_t index);
    static_object(native_vector<object_ptr> &&data);
    static_object(native_vector<object_ptr> &&data, size_t index);

    /* behavior::objectable */
    native_bool equal(object const &o) const;
    void to_string(fmt::memory_buffer &buff) const;
    native_persistent_string to_string() const;
    native_hash to_hash();

    /* behavior::seqable */
    native_box<static_object> seq();
    native_box<static_object> fresh_seq();

    /* behavior::countable */
    size_t count() const;

    /* behavior::sequence */
    object_ptr first() const;
    native_box<static_object> next() const;
    obj::cons_ptr cons(object_ptr head);

    /* behavior::sequenceable_in_place */
    native_box<static_object> next_in_place();
    object_ptr next_in_place_first();

    object base{ object_type::native_vector_sequence };
    native_vector<object_ptr> data{};
    size_t index{};
  };

  namespace obj
  {
    using native_vector_sequence = static_object<object_type::native_vector_sequence>;
    using native_vector_sequence_ptr = native_box<native_vector_sequence>;
  }
}
