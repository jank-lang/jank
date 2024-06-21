#pragma once

namespace jank::runtime
{
  template <>
  struct static_object<object_type::transient_vector> : gc
  {
    static constexpr bool pointer_free{ false };

    using value_type = detail::native_transient_vector;
    using persistent_type = static_object<object_type::persistent_vector>;

    static_object() = default;
    static_object(static_object &&) noexcept = default;
    static_object(static_object const &) = default;
    static_object(detail::native_persistent_vector const &d);
    static_object(detail::native_persistent_vector &&d);
    static_object(value_type &&d);

    static native_box<static_object> empty()
    {
      return make_box<static_object>();
    }

    /* behavior::objectable */
    native_bool equal(object const &) const;
    native_persistent_string to_string() const;
    void to_string(fmt::memory_buffer &buff) const;
    native_hash to_hash() const;

    /* behavior::countable */
    size_t count() const;

    /* behavior::conjable_in_place */
    native_box<static_object> cons_in_place(object_ptr head);

    /* behavior::persistentable */
    native_box<persistent_type> to_persistent();

    /* behavior::callable */
    object_ptr call(object_ptr const);

    /* behavior::associatively_readable */
    object_ptr get(object_ptr const idx) const;
    object_ptr get(object_ptr const idx, object_ptr const fallback) const;
    object_ptr get_entry(object_ptr const idx) const;
    native_bool contains(object_ptr const elem) const;

    native_box<static_object> pop_in_place();

    void assert_active() const;

    object base{ object_type::transient_vector };
    value_type data;
    mutable native_hash hash{};
    native_bool active{ true };
  };

  namespace obj
  {
    using transient_vector = static_object<object_type::transient_vector>;
    using transient_vector_ptr = native_box<transient_vector>;
  }
}
