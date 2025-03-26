#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/detail/type.hpp>

namespace jank::runtime::obj
{
  using transient_sorted_set_ptr = native_box<struct transient_sorted_set>;

  struct transient_sorted_set : gc
  {
    static constexpr object_type obj_type{ object_type::transient_sorted_set };
    static constexpr bool pointer_free{ false };

    using value_type = runtime::detail::native_transient_sorted_set;
    using persistent_type_ptr = native_box<struct persistent_sorted_set>;

    transient_sorted_set() = default;
    transient_sorted_set(transient_sorted_set &&) noexcept = default;
    transient_sorted_set(transient_sorted_set const &) = default;
    transient_sorted_set(runtime::detail::native_persistent_sorted_set const &d);
    transient_sorted_set(runtime::detail::native_persistent_sorted_set &&d);
    transient_sorted_set(value_type &&d);

    static transient_sorted_set_ptr empty();

    /* behavior::object_like */
    native_bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    void to_string(util::string_builder &buff) const;
    jtl::immutable_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::countable */
    size_t count() const;

    /* behavior::conjable_in_place */
    transient_sorted_set_ptr conj_in_place(object_ptr elem);

    /* behavior::persistentable */
    persistent_type_ptr to_persistent();

    /* behavior::callable */
    object_ptr call(object_ptr const);
    object_ptr call(object_ptr const, object_ptr const fallback);

    /* behavior::associatively_readable */
    object_ptr get(object_ptr const elem);
    object_ptr get(object_ptr const elem, object_ptr const fallback);
    object_ptr get_entry(object_ptr const elem);
    native_bool contains(object_ptr const elem) const;

    transient_sorted_set_ptr disjoin_in_place(object_ptr const elem);

    void assert_active() const;

    object base{ obj_type };
    value_type data;
    mutable native_hash hash{};
    native_bool active{ true };
  };
}
