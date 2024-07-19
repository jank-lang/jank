#pragma once

namespace jank::runtime
{
  template <>
  struct static_object<object_type::native_array_sequence> : gc
  {
    static constexpr native_bool pointer_free{ false };
    static constexpr native_bool is_sequential{ true };

    static_object() = delete;
    static_object(static_object &&) = default;
    static_object(static_object const &) = default;
    static_object(object_ptr * const arr, size_t const size);
    static_object(object_ptr * const arr, size_t const index, size_t const size);

    template <typename... Args>
    static_object(object_ptr const first, Args const... rest)
      : arr{ make_array_box<object_ptr>(first, rest...) }
      , size{ sizeof...(Args) + 1 }
    {
    }

    /* behavior::object_like */
    native_bool equal(object const &o) const;
    void to_string(fmt::memory_buffer &buff) const;
    native_persistent_string to_string() const;
    native_hash to_hash() const;

    /* behavior::seqable */
    native_box<static_object> seq();
    native_box<static_object> fresh_seq();

    /* behavior::countable */
    size_t count() const;

    /* behavior::sequence */
    object_ptr first() const;
    native_box<static_object> next() const;
    obj::cons_ptr conj(object_ptr head);

    /* behavior::sequenceable_in_place */
    native_box<static_object> next_in_place();

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
