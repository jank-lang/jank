#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/detail/type.hpp>

namespace jank::runtime::obj
{
  using transient_hash_set_ref = oref<struct transient_hash_set>;

  struct transient_hash_set : gc
  {
    static constexpr object_type obj_type{ object_type::transient_hash_set };
    static constexpr bool pointer_free{ false };

    using value_type = runtime::detail::native_transient_hash_set;
    using persistent_type_ref = oref<struct persistent_hash_set>;

    transient_hash_set() = default;
    transient_hash_set(transient_hash_set &&) noexcept = default;
    transient_hash_set(transient_hash_set const &) = default;
    transient_hash_set(runtime::detail::native_persistent_hash_set const &d);
    transient_hash_set(runtime::detail::native_persistent_hash_set &&d);
    transient_hash_set(value_type &&d);

    static transient_hash_set_ref empty();

    /* behavior::object_like */
    native_bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    void to_string(util::string_builder &buff) const;
    jtl::immutable_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::countable */
    size_t count() const;

    /* behavior::conjable_in_place */
    transient_hash_set_ref conj_in_place(object_ref elem);

    /* behavior::persistentable */
    persistent_type_ref to_persistent();

    /* behavior::callable */
    object_ref call(object_ref const) const;
    object_ref call(object_ref const, object_ref const fallback) const;

    /* behavior::associatively_readable */
    object_ref get(object_ref const elem) const;
    object_ref get(object_ref const elem, object_ref const fallback) const;
    object_ref get_entry(object_ref const elem) const;
    native_bool contains(object_ref const elem) const;

    transient_hash_set_ref disjoin_in_place(object_ref const elem);

    void assert_active() const;

    object base{ obj_type };
    value_type data;
    mutable native_hash hash{};
    native_bool active{ true };
  };
}
