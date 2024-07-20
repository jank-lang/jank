#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/obj/persistent_sorted_map_sequence.hpp>
#include <jank/runtime/obj/detail/base_persistent_map.hpp>

namespace jank::runtime
{
  namespace obj
  {
    using transient_sorted_map = static_object<object_type::transient_sorted_map>;
    using transient_sorted_map_ptr = native_box<transient_sorted_map>;
  }

  template <>
  struct static_object<object_type::persistent_sorted_map>
    : obj::detail::base_persistent_map<object_type::persistent_sorted_map,
                                       object_type::persistent_sorted_map_sequence,
                                       runtime::detail::native_persistent_sorted_map>
  {
    using transient_type = static_object<object_type::transient_sorted_map>;

    static_object() = default;
    static_object(static_object &&) = default;
    static_object(static_object const &) = default;
    static_object(value_type &&d);
    static_object(value_type const &d);
    static_object(object_ptr meta, value_type &&d);

    template <typename... Args>
    static_object(runtime::detail::in_place_unique, Args &&...args)
      : data{ std::forward<Args>(args)... }
    {
    }

    template <typename... Args>
    static_object(object_ptr const meta, runtime::detail::in_place_unique, Args &&...args)
      : data{ std::forward<Args>(args)... }
    {
      this->meta = meta;
    }

    static native_box<static_object> empty()
    {
      static auto const ret(make_box<static_object>());
      return ret;
    }

    using base_persistent_map::base_persistent_map;

    template <typename... Args>
    static native_box<static_object> create_unique(Args &&...pairs)
    {
      return make_box<static_object>(runtime::detail::in_place_unique{},
                                     std::forward<Args>(pairs)...);
    }

    template <typename... Args>
    static native_box<static_object> create_unique_with_meta(object_ptr const meta, Args &&...pairs)
    {
      return make_box<static_object>(meta,
                                     runtime::detail::in_place_unique{},
                                     std::forward<Args>(pairs)...);
    }

    static native_box<static_object> create_from_seq(object_ptr const seq);

    /* behavior::associatively_readable */
    object_ptr get(object_ptr const key) const;
    object_ptr get(object_ptr const key, object_ptr const fallback) const;
    object_ptr get_entry(object_ptr key) const;
    native_bool contains(object_ptr key) const;

    /* behavior::associatively_writable */
    native_box<static_object> assoc(object_ptr key, object_ptr val) const;
    native_box<static_object> dissoc(object_ptr key) const;

    /* behavior::conjable */
    native_box<static_object> conj(object_ptr head) const;

    /* behavior::callable */
    object_ptr call(object_ptr);
    object_ptr call(object_ptr, object_ptr);

    /* behavior::transientable */
    obj::transient_sorted_map_ptr to_transient() const;

    value_type data{};
  };

  namespace obj
  {
    using persistent_sorted_map = static_object<object_type::persistent_sorted_map>;
    using persistent_sorted_map_ptr = native_box<persistent_sorted_map>;
  }
}
