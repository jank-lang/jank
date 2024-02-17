#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime
{
  template <>
  struct static_object<object_type::transient_hash_map> : gc
  {
    static constexpr bool pointer_free{ false };

    using value_type = detail::native_transient_hash_map;
    using persistent_type = static_object<object_type::persistent_hash_map>;

    static_object() = default;
    static_object(static_object &&) = default;
    static_object(static_object const &) = default;
    static_object(detail::native_persistent_hash_map const &d);
    static_object(detail::native_persistent_hash_map &&d);
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

    /* behavior::associatively_readable */
    object_ptr get(object_ptr const key) const;
    object_ptr get(object_ptr const key, object_ptr const fallback) const;
    object_ptr get_entry(object_ptr key) const;
    native_bool contains(object_ptr key) const;

    /* behavior::associatively_writable_in_place */
    native_box<static_object> assoc_in_place(object_ptr key, object_ptr val);

    /* behavior::consable_in_place */
    native_box<static_object> cons_in_place(object_ptr head);

    /* behavior::persistentable */
    native_box<persistent_type> to_persistent();

    /* behavior::callable */
    object_ptr call(object_ptr) const;
    object_ptr call(object_ptr, object_ptr) const;

    void assert_active() const;

    object base{ object_type::transient_hash_map };
    value_type data;
    mutable native_hash hash{};
    native_bool active{ true };
  };

  namespace obj
  {
    using transient_hash_map = static_object<object_type::transient_hash_map>;
    using transient_hash_map_ptr = native_box<transient_hash_map>;
  }
}
