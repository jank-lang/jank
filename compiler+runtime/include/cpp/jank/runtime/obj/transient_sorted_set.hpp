#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/detail/type.hpp>

namespace jank::runtime::obj
{
  using transient_sorted_set_ref = oref<struct transient_sorted_set>;

  struct transient_sorted_set : object
  {
    static constexpr object_type obj_type{ object_type::transient_sorted_set };
    static constexpr object_behavior obj_behaviors{ object_behavior::call | object_behavior::get };
    static constexpr bool pointer_free{ false };

    using value_type = runtime::detail::native_transient_sorted_set;
    using persistent_type_ref = oref<struct persistent_sorted_set>;

    transient_sorted_set();
    transient_sorted_set(transient_sorted_set &&) noexcept = default;
    transient_sorted_set(transient_sorted_set const &) = default;
    transient_sorted_set(value_type const &d);
    transient_sorted_set(value_type &&d);

    static transient_sorted_set_ref empty();

    /* behavior::countable */
    usize count() const;

    /* behavior::conjable_in_place */
    transient_sorted_set_ref conj_in_place(object_ref const elem);

    /* behavior::persistentable */
    persistent_type_ref to_persistent();

    /* behavior::callable */
    using object::call;
    object_ref call(object_ref const) const override;
    object_ref call(object_ref const, object_ref const fallback) const override;

    /* behavior::get */
    object_ref get(object_ref const elem) const override;
    object_ref get(object_ref const elem, object_ref const fallback) const override;
    bool contains(object_ref const elem) const override;

    transient_sorted_set_ref disjoin_in_place(object_ref const elem);

    void assert_active() const;

    /*** XXX: Everything here is not thread-safe, but not shared. ***/
    value_type data;
    bool active{ true };
  };
}
