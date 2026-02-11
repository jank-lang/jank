#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/detail/type.hpp>

namespace jank::runtime::obj
{
  using transient_hash_set_ref = oref<struct transient_hash_set>;

  struct transient_hash_set : object
  {
    static constexpr object_type obj_type{ object_type::transient_hash_set };
    static constexpr object_behavior obj_behaviors{ object_behavior::call | object_behavior::get };
    static constexpr bool pointer_free{ false };

    using value_type = runtime::detail::native_transient_hash_set;
    using persistent_type_ref = oref<struct persistent_hash_set>;

    transient_hash_set();
    transient_hash_set(transient_hash_set &&) noexcept = default;
    transient_hash_set(transient_hash_set const &) = default;
    transient_hash_set(runtime::detail::native_persistent_hash_set const &d);
    transient_hash_set(runtime::detail::native_persistent_hash_set &&d);
    transient_hash_set(value_type &&d);

    static transient_hash_set_ref empty();

    /* behavior::countable */
    usize count() const;

    /* behavior::conjable_in_place */
    transient_hash_set_ref conj_in_place(object_ref const elem);

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

    transient_hash_set_ref disjoin_in_place(object_ref const elem);

    void assert_active() const;

    /*** XXX: Everything here is not thread-safe, but not shared. ***/
    value_type data;
    bool active{ true };
  };
}
