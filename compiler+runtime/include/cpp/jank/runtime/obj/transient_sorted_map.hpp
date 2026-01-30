#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/detail/type.hpp>

namespace jank::runtime::obj
{
  using transient_sorted_map_ref = oref<struct transient_sorted_map>;

  struct transient_sorted_map : object
  {
    static constexpr object_type obj_type{ object_type::transient_sorted_map };
    static constexpr object_behavior obj_behaviors{ object_behavior::call | object_behavior::get
                                                    | object_behavior::find };
    static constexpr bool pointer_free{ false };

    using value_type = runtime::detail::native_transient_sorted_map;
    using persistent_type_ref = oref<struct persistent_sorted_map>;

    transient_sorted_map();
    transient_sorted_map(transient_sorted_map &&) noexcept = default;
    transient_sorted_map(transient_sorted_map const &) = default;
    transient_sorted_map(value_type const &d);
    transient_sorted_map(value_type &&d);

    static transient_sorted_map_ref empty();

    /* behavior::countable */
    usize count() const;

    /* behavior::get */
    object_ref get(object_ref const key) const override;
    object_ref get(object_ref const key, object_ref const fallback) const override;
    bool contains(object_ref const key) const override;

    /* behavior::find */
    object_ref find(object_ref const key) const override;

    /* behavior::associatively_writable_in_place */
    transient_sorted_map_ref assoc_in_place(object_ref const key, object_ref const val);
    transient_sorted_map_ref dissoc_in_place(object_ref const key);

    /* behavior::conjable_in_place */
    transient_sorted_map_ref conj_in_place(object_ref const head);

    /* behavior::persistentable */
    persistent_type_ref to_persistent();

    /* behavior::callable */
    using object::call;
    object_ref call(object_ref const) const override;
    object_ref call(object_ref const, object_ref const) const override;

    void assert_active() const;

    /*** XXX: Everything here is not thread-safe, but not shared. ***/
    value_type data;
    bool active{ true };
  };
}
