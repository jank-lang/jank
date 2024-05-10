#pragma once

namespace jank::runtime
{
  template <>
  struct static_object<object_type::transient_set> : gc
  {
    static constexpr bool pointer_free{ false };

    using value_type = detail::native_transient_set;
    using persistent_type = static_object<object_type::persistent_set>;

    static_object() = default;
    static_object(static_object &&) noexcept = default;
    static_object(static_object const &) = default;
    static_object(detail::native_persistent_set const &d);
    static_object(detail::native_persistent_set &&d);
    static_object(value_type &&d);

    static native_box<static_object> empty()
    {
      static auto const ret(make_box<static_object>());
      return ret;
    }

    /* behavior::objectable */
    native_bool equal(object const &) const;
    native_persistent_string to_string() const;
    void to_string(fmt::memory_buffer &buff) const;
    native_hash to_hash() const;

    /* behavior::countable */
    size_t count() const;

    /* behavior::conjable_in_place */
    native_box<static_object> cons_in_place(object_ptr elem);

    /* behavior::persistentable */
    native_box<persistent_type> to_persistent();

    /* behavior::callable */
    object_ptr call(object_ptr const) const;
    object_ptr call(object_ptr const, object_ptr const fallback) const;

    /* behavior::associatively_readable */
    object_ptr get(object_ptr const elem) const;
    object_ptr get(object_ptr const elem, object_ptr const fallback) const;
    object_ptr get_entry(object_ptr const elem) const;
    native_bool contains(object_ptr const elem) const;

    native_box<static_object> disjoin_in_place(object_ptr const elem);

    void assert_active() const;

    object base{ object_type::transient_set };
    value_type data;
    mutable native_hash hash{};
    native_bool active{ true };
  };

  namespace obj
  {
    using transient_set = static_object<object_type::transient_set>;
    using transient_set_ptr = native_box<transient_set>;
  }
}
