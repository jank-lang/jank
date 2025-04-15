#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/detail/type.hpp>

namespace jank::runtime::detail
{
  struct native_persistent_array_map;
}

namespace jank::runtime::obj
{
  using transient_hash_map_ref = oref<struct transient_hash_map>;

  struct transient_hash_map : gc
  {
    static constexpr object_type obj_type{ object_type::transient_hash_map };
    static constexpr bool pointer_free{ false };

    using value_type = runtime::detail::native_transient_hash_map;
    using persistent_type_ref = oref<struct persistent_hash_map>;

    transient_hash_map() = default;
    transient_hash_map(transient_hash_map &&) noexcept = default;
    transient_hash_map(transient_hash_map const &) = default;
    transient_hash_map(runtime::detail::native_persistent_hash_map const &d);
    transient_hash_map(runtime::detail::native_persistent_hash_map &&d);
    transient_hash_map(runtime::detail::native_persistent_array_map const &m);
    transient_hash_map(value_type &&d);

    static transient_hash_map_ref empty();

    /* behavior::object_like */
    native_bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    void to_string(util::string_builder &buff) const;
    jtl::immutable_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::countable */
    size_t count() const;

    /* behavior::associatively_readable */
    object_ref get(object_ref const key) const;
    object_ref get(object_ref const key, object_ref const fallback) const;
    object_ref get_entry(object_ref key) const;
    native_bool contains(object_ref key) const;

    /* behavior::associatively_writable_in_place */
    transient_hash_map_ref assoc_in_place(object_ref const key, object_ref const val);
    transient_hash_map_ref dissoc_in_place(object_ref const key);

    /* behavior::conjable_in_place */
    transient_hash_map_ref conj_in_place(object_ref head);

    /* behavior::persistentable */
    persistent_type_ref to_persistent();

    /* behavior::callable */
    object_ref call(object_ref) const;
    object_ref call(object_ref, object_ref) const;

    void assert_active() const;

    object base{ obj_type };
    value_type data;
    mutable native_hash hash{};
    native_bool active{ true };
  };
}
