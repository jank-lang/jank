#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/obj/persistent_sorted_map_sequence.hpp>
#include <jank/runtime/obj/detail/base_persistent_map.hpp>
#include <jank/runtime/detail/native_array_map.hpp>

namespace jank::runtime::obj
{
  using transient_sorted_map_ref = oref<struct transient_sorted_map>;
  using persistent_sorted_map_ref = oref<struct persistent_sorted_map>;

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
    persistent_sorted_map(object_ref meta, value_type &&d);
    persistent_sorted_map(jtl::option<object_ref> const &meta, value_type &&d);

    template <typename... Args>
    persistent_sorted_map(runtime::detail::in_place_unique, Args &&...args)
      : data{ std::forward<Args>(args)... }
    {
    }

    template <typename... Args>
    persistent_sorted_map(object_ref const meta, runtime::detail::in_place_unique, Args &&...args)
      : data{ std::forward<Args>(args)... }
    {
      this->meta = meta;
    }

    static persistent_sorted_map_ref empty();

    using base_persistent_map::base_persistent_map;

    template <typename... Args>
    static persistent_sorted_map_ref create_unique(Args &&...pairs)
    {
      return make_box<persistent_sorted_map>(runtime::detail::in_place_unique{},
                                             std::forward<Args>(pairs)...);
    }

    template <typename... Args>
    static persistent_sorted_map_ref create_unique_with_meta(object_ref const meta, Args &&...pairs)
    {
      return make_box<persistent_sorted_map>(meta,
                                             runtime::detail::in_place_unique{},
                                             std::forward<Args>(pairs)...);
    }

    static persistent_sorted_map_ref create_from_seq(object_ref const seq);

    /* behavior::associatively_readable */
    object_ref get(object_ref const key) const;
    object_ref get(object_ref const key, object_ref const fallback) const;
    object_ref get_entry(object_ref key) const;
    bool contains(object_ref key) const;

    /* behavior::associatively_writable */
    persistent_sorted_map_ref assoc(object_ref key, object_ref val) const;
    persistent_sorted_map_ref dissoc(object_ref key) const;

    /* behavior::callable */
    object_ref call(object_ref) const;
    object_ref call(object_ref, object_ref) const;

    /* behavior::transientable */
    obj::transient_sorted_map_ref to_transient() const;

    value_type data{};
  };
}
