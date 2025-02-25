#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/obj/persistent_sorted_map_sequence.hpp>
#include <jank/runtime/obj/detail/base_persistent_map.hpp>
#include <jank/runtime/detail/native_persistent_array_map.hpp>

namespace jank::runtime::obj
{
  using transient_sorted_map_ptr = native_box<struct transient_sorted_map>;
  using persistent_sorted_map_ptr = native_box<struct persistent_sorted_map>;

  struct persistent_sorted_map
    : obj::detail::base_persistent_map<persistent_sorted_map,
                                       persistent_sorted_map_sequence,
                                       runtime::detail::native_persistent_sorted_map>
  {
    static constexpr object_type obj_type{ object_type::persistent_sorted_map };

    using transient_type = transient_sorted_map;
    using parent_type
      = obj::detail::base_persistent_map<persistent_sorted_map,
                                         persistent_sorted_map_sequence,
                                         runtime::detail::native_persistent_sorted_map>;

    persistent_sorted_map() = default;
    persistent_sorted_map(persistent_sorted_map &&) noexcept = default;
    persistent_sorted_map(persistent_sorted_map const &) = default;
    persistent_sorted_map(value_type &&d);
    persistent_sorted_map(value_type const &d);
    persistent_sorted_map(object_ptr meta, value_type &&d);
    persistent_sorted_map(option<object_ptr> const &meta, value_type &&d);

    template <typename... Args>
    persistent_sorted_map(runtime::detail::in_place_unique, Args &&...args)
      : data{ std::forward<Args>(args)... }
    {
    }

    template <typename... Args>
    persistent_sorted_map(object_ptr const meta, runtime::detail::in_place_unique, Args &&...args)
      : data{ std::forward<Args>(args)... }
    {
      this->meta = meta;
    }

    static persistent_sorted_map_ptr empty()
    {
      static auto const ret(make_box<persistent_sorted_map>());
      return ret;
    }

    using base_persistent_map::base_persistent_map;

    template <typename... Args>
    static persistent_sorted_map_ptr create_unique(Args &&...pairs)
    {
      return make_box<persistent_sorted_map>(runtime::detail::in_place_unique{},
                                             std::forward<Args>(pairs)...);
    }

    template <typename... Args>
    static persistent_sorted_map_ptr create_unique_with_meta(object_ptr const meta, Args &&...pairs)
    {
      return make_box<persistent_sorted_map>(meta,
                                             runtime::detail::in_place_unique{},
                                             std::forward<Args>(pairs)...);
    }

    static persistent_sorted_map_ptr create_from_seq(object_ptr const seq);

    /* behavior::associatively_readable */
    object_ptr get(object_ptr const key) const;
    object_ptr get(object_ptr const key, object_ptr const fallback) const;
    object_ptr get_entry(object_ptr key) const;
    native_bool contains(object_ptr key) const;

    /* behavior::associatively_writable */
    persistent_sorted_map_ptr assoc(object_ptr key, object_ptr val) const;
    persistent_sorted_map_ptr dissoc(object_ptr key) const;

    /* behavior::callable */
    object_ptr call(object_ptr) const;
    object_ptr call(object_ptr, object_ptr) const;

    /* behavior::transientable */
    obj::transient_sorted_map_ptr to_transient() const;

    value_type data{};
  };
}
