#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/detail/native_persistent_array_map.hpp>
#include <jank/runtime/obj/persistent_array_map_sequence.hpp>
#include <jank/runtime/obj/detail/base_persistent_map.hpp>

namespace jank::runtime::obj
{
  using persistent_array_map_ptr = native_box<struct persistent_array_map>;
  using transient_hash_map_ptr = native_box<struct transient_hash_map>;

  struct persistent_array_map
    : obj::detail::base_persistent_map<persistent_array_map,
                                       persistent_array_map_sequence,
                                       runtime::detail::native_persistent_array_map>
  {
    static constexpr object_type obj_type{ object_type::persistent_array_map };
    static constexpr size_t max_size{ value_type::max_size };

    persistent_array_map() = default;
    persistent_array_map(persistent_array_map &&) noexcept = default;
    persistent_array_map(persistent_array_map const &) = default;
    persistent_array_map(value_type &&d);
    persistent_array_map(value_type const &d);
    persistent_array_map(object_ptr meta, value_type &&d);

    template <typename... Args>
    persistent_array_map(runtime::detail::in_place_unique, Args &&...args)
      : data{ runtime::detail::in_place_unique{}, std::forward<Args>(args)... }
    {
    }

    template <typename... Args>
    persistent_array_map(object_ptr const meta, runtime::detail::in_place_unique, Args &&...args)
      : data{ runtime::detail::in_place_unique{}, std::forward<Args>(args)... }
    {
      this->meta = meta;
    }

    static persistent_array_map_ptr empty()
    {
      static auto const ret(make_box<persistent_array_map>());
      return ret;
    }

    using base_persistent_map::base_persistent_map;

    template <typename... Args>
    static persistent_array_map_ptr create_unique(Args &&...args)
    {
      return make_box<persistent_array_map>(runtime::detail::in_place_unique{},
                                            make_array_box<object_ptr>(std::forward<Args>(args)...),
                                            sizeof...(args));
    }

    template <typename... Args>
    static persistent_array_map_ptr create_unique_with_meta(object_ptr const meta, Args &&...args)
    {
      return make_box<persistent_array_map>(meta,
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
    persistent_array_map_ptr dissoc(object_ptr key) const;

    /* behavior::conjable */
    object_ptr conj(object_ptr head) const;

    /* behavior::callable */
    object_ptr call(object_ptr) const;
    object_ptr call(object_ptr, object_ptr) const;

    /* behavior::transientable */
    transient_hash_map_ptr to_transient() const;

    value_type data{};
  };
}
