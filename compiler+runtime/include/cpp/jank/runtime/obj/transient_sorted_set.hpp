#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/detail/type.hpp>

namespace jank::runtime::obj
{
  using transient_sorted_set_ref = oref<struct transient_sorted_set>;

  struct transient_sorted_set : gc
  {
    static constexpr object_type obj_type{ object_type::transient_sorted_set };
    static constexpr bool pointer_free{ false };

    using value_type = runtime::detail::native_transient_sorted_set;
    using persistent_type_ref = oref<struct persistent_sorted_set>;

    transient_sorted_set() = default;
    transient_sorted_set(transient_sorted_set &&) noexcept = default;
    transient_sorted_set(transient_sorted_set const &) = default;
    transient_sorted_set(runtime::detail::native_persistent_sorted_set const &d);
    transient_sorted_set(runtime::detail::native_persistent_sorted_set &&d);
    transient_sorted_set(value_type &&d);

    static transient_sorted_set_ref empty();

    /* behavior::object_like */
    bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    void to_string(util::string_builder &buff) const;
    jtl::immutable_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::countable */
    usize count() const;

    /* behavior::conjable_in_place */
    transient_sorted_set_ref conj_in_place(object_ref elem);

    /* behavior::persistentable */
    persistent_type_ref to_persistent();

    /* behavior::callable */
    object_ref call(object_ref const);
    object_ref call(object_ref const, object_ref const fallback);

    /* behavior::associatively_readable */
    object_ref get(object_ref const elem);
    object_ref get(object_ref const elem, object_ref const fallback);
    object_ref get_entry(object_ref const elem);
    bool contains(object_ref const elem) const;

    transient_sorted_set_ref disjoin_in_place(object_ref const elem);

    void assert_active() const;

    object base{ obj_type };
    value_type data;
    mutable native_hash hash{};
    bool active{ true };
  };
}
