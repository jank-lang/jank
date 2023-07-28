#pragma once

namespace jank::runtime
{
  template <>
  struct static_object<object_type::native_array_sequence> : gc
  {
    static constexpr bool pointer_free{ false };

    static_object() = default;
    static_object(static_object &&) = default;
    static_object(static_object const &) = default;
    static_object(object_ptr * const arr, size_t const size);
    static_object(object_ptr * const arr, size_t const index, size_t const size);
    template <typename ...Args>
    static_object(object_ptr first, Args ... rest)
      : arr{ make_array_box<object_ptr>(first, rest...) }, size{ sizeof...(Args) + 1 }
    { }

    /* behavior::objectable */
    native_bool equal(object const &o) const;
    void to_string(fmt::memory_buffer &buff) const;
    native_string to_string() const;
    native_integer to_hash();

    /* behavior::seqable */
    native_box<static_object> seq();
    native_box<static_object> fresh_seq();

    /* behavior::countable */
    size_t count() const;

    /* behavior::sequence */
    object_ptr first() const;
    native_box<static_object> next() const;
    native_box<static_object> next_in_place();
    object_ptr next_in_place_first();
    obj::cons_ptr cons(object_ptr head);

    object base{ object_type::native_array_sequence };
    object_ptr *arr{};
    size_t index{};
    size_t size{};
  };

  namespace obj
  {
    using native_array_sequence = static_object<object_type::native_array_sequence>;
    using native_array_sequence_ptr = native_box<native_array_sequence>;
  }
}
