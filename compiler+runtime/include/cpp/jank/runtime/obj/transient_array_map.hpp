#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/detail/native_array_map.hpp>

namespace jank::runtime::obj
{
  using transient_array_map_ref = oref<struct transient_array_map>;

  struct transient_array_map : gc
  {
    static constexpr object_type obj_type{ object_type::transient_array_map };
    static constexpr bool pointer_free{ false };

    using value_type = runtime::detail::native_array_map;
    using persistent_type_ref = oref<struct persistent_array_map>;

    transient_array_map() = default;
    transient_array_map(transient_array_map &&) noexcept = default;
    transient_array_map(transient_array_map const &) = default;
    transient_array_map(runtime::detail::native_array_map const &d);
    transient_array_map(runtime::detail::native_array_map &&d);

    static transient_array_map_ref empty();

    /* behavior::object_like */
    bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    void to_string(util::string_builder &buff) const;
    jtl::immutable_string to_code_string() const;
    uhash to_hash() const;

    /* behavior::countable */
    usize count() const;

    /* behavior::associatively_readable */
    object_ref get(object_ref const key) const;
    object_ref get(object_ref const key, object_ref const fallback) const;
    object_ref get_entry(object_ref key) const;
    bool contains(object_ref key) const;

    /* behavior::associatively_writable_in_place */
    object_ref assoc_in_place(object_ref const key, object_ref const val);
    transient_array_map_ref dissoc_in_place(object_ref const key);

    /* behavior::conjable_in_place */
    object_ref conj_in_place(object_ref head);

    /* behavior::persistentable */
    persistent_type_ref to_persistent();

    /* behavior::callable */
    object_ref call(object_ref) const;
    object_ref call(object_ref, object_ref) const;

    void assert_active() const;

    object base{ obj_type };
    value_type data;
    mutable uhash hash{};
    bool active{ true };
  };
}
