#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/detail/native_persistent_array_map.hpp>
#include <jank/runtime/obj/persistent_array_map_sequence.hpp>
#include <jank/runtime/obj/detail/base_persistent_map.hpp>

namespace jank::runtime
{
  template <>
  struct static_object<object_type::persistent_array_map>
    : obj::detail::base_persistent_map<object_type::persistent_array_map,
                                       object_type::persistent_array_map_sequence,
                                       runtime::detail::native_persistent_array_map>
  {
    static constexpr size_t max_size{ value_type::max_size };

    static_object() = default;
    static_object(static_object &&) = default;
    static_object(static_object const &) = default;
    static_object(value_type &&d);
    static_object(value_type const &d);
    static_object(object_ptr meta, value_type &&d);

    template <typename... Args>
    static_object(runtime::detail::in_place_unique, Args &&...args)
      : data{ runtime::detail::in_place_unique{}, std::forward<Args>(args)... }
    {
    }

    template <typename... Args>
    static_object(object_ptr const meta, runtime::detail::in_place_unique, Args &&...args)
      : data{ runtime::detail::in_place_unique{}, std::forward<Args>(args)... }
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
    static native_box<static_object> create_unique(Args &&...args)
    {
      return make_box<static_object>(runtime::detail::in_place_unique{},
                                     make_array_box<object_ptr>(std::forward<Args>(args)...),
                                     sizeof...(args));
    }

    template <typename... Args>
    static native_box<static_object> create_unique_with_meta(object_ptr const meta, Args &&...args)
    {
      return make_box<static_object>(meta,
                                     runtime::detail::in_place_unique{},
                                     make_array_box<object_ptr>(std::forward<Args>(args)...),
                                     sizeof...(args));
    }

    /* behavior::associatively_readable */
    object_ptr get(object_ptr const key) const;
    object_ptr get(object_ptr const key, object_ptr const fallback) const;
    object_ptr get_entry(object_ptr key) const;
    native_bool contains(object_ptr key) const;

    /* behavior::associatively_writable */
    object_ptr assoc(object_ptr key, object_ptr val) const;
    native_box<static_object> dissoc(object_ptr key) const;

    /* behavior::conjable */
    object_ptr conj(object_ptr head) const;

    /* behavior::callable */
    object_ptr call(object_ptr) const;
    object_ptr call(object_ptr, object_ptr) const;

    value_type data{};
  };

  namespace obj
  {
    using persistent_array_map = static_object<object_type::persistent_array_map>;
    using persistent_array_map_ptr = native_box<persistent_array_map>;
  }
}
