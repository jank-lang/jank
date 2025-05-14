#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/detail/native_persistent_array_map.hpp>
#include <jank/runtime/obj/persistent_array_map_sequence.hpp>
#include <jank/runtime/obj/detail/base_persistent_map.hpp>

namespace jank::runtime::obj
{
  using persistent_array_map_ref = oref<struct persistent_array_map>;
  using transient_hash_map_ref = oref<struct transient_hash_map>;

  struct persistent_array_map
    : obj::detail::base_persistent_map<persistent_array_map,
                                       persistent_array_map_sequence,
                                       runtime::detail::native_persistent_array_map>
  {
    static constexpr object_type obj_type{ object_type::persistent_array_map };
    static constexpr usize max_size{ value_type::max_size };
    using parent_type
      = obj::detail::base_persistent_map<persistent_array_map,
                                         persistent_array_map_sequence,
                                         runtime::detail::native_persistent_array_map>;


    persistent_array_map() = default;
    persistent_array_map(persistent_array_map &&) noexcept = default;
    persistent_array_map(persistent_array_map const &) = default;
    persistent_array_map(value_type &&d);
    persistent_array_map(value_type const &d);
    persistent_array_map(jtl::option<object_ref> const &meta, value_type &&d);

    template <typename... Args>
    persistent_array_map(runtime::detail::in_place_unique, Args &&...args)
      : data{ runtime::detail::in_place_unique{}, std::forward<Args>(args)... }
    {
    }

    template <typename... Args>
    persistent_array_map(object_ref const meta, runtime::detail::in_place_unique, Args &&...args)
      : data{ runtime::detail::in_place_unique{}, std::forward<Args>(args)... }
    {
      this->meta = meta;
    }

    static persistent_array_map_ref empty()
    {
      static auto const ret(make_box<persistent_array_map>());
      return ret;
    }

    using base_persistent_map::base_persistent_map;

    template <typename... Args>
    static persistent_array_map_ref create_unique(Args &&...args)
    {
      return make_box<persistent_array_map>(runtime::detail::in_place_unique{},
                                            make_array_box<object_ref>(std::forward<Args>(args)...),
                                            sizeof...(args));
    }

    template <typename... Args>
    static persistent_array_map_ref create_unique_with_meta(object_ref const meta, Args &&...args)
    {
      return make_box<persistent_array_map>(meta,
                                            runtime::detail::in_place_unique{},
                                            make_array_box<object_ref>(std::forward<Args>(args)...),
                                            sizeof...(args));
    }

    /* behavior::associatively_readable */
    object_ref get(object_ref const key) const;
    object_ref get(object_ref const key, object_ref const fallback) const;
    object_ref get_entry(object_ref key) const;
    bool contains(object_ref key) const;

    /* behavior::associatively_writable */
    object_ref assoc(object_ref key, object_ref val) const;
    persistent_array_map_ref dissoc(object_ref key) const;

    /* behavior::callable */
    object_ref call(object_ref) const;
    object_ref call(object_ref, object_ref) const;

    /* behavior::transientable */
    transient_hash_map_ref to_transient() const;

    value_type data{};
  };
}
