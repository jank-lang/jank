#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/detail/type.hpp>

namespace jank::runtime::obj
{
  using transient_hash_set_ptr = native_box<struct transient_hash_set>;

  struct transient_hash_set : gc
  {
    static constexpr object_type obj_type{ object_type::transient_hash_set };
    static constexpr bool pointer_free{ false };

    using value_type = runtime::detail::native_transient_hash_set;
    using persistent_type_ptr = native_box<struct persistent_hash_set>;

    transient_hash_set() = default;
    transient_hash_set(transient_hash_set &&) noexcept = default;
    transient_hash_set(transient_hash_set const &) = default;
    transient_hash_set(runtime::detail::native_persistent_hash_set const &d);
    transient_hash_set(runtime::detail::native_persistent_hash_set &&d);
    transient_hash_set(value_type &&d);

    static transient_hash_set_ptr empty();

    /* behavior::object_like */
    native_bool equal(object const &) const;
    native_persistent_string to_string() const;
    void to_string(util::string_builder &buff) const;
    native_persistent_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::countable */
    size_t count() const;

    /* behavior::conjable_in_place */
    transient_hash_set_ptr conj_in_place(object_ptr elem);

    /* behavior::persistentable */
    persistent_type_ptr to_persistent();

    /* behavior::callable */
    object_ptr call(object_ptr const) const;
    object_ptr call(object_ptr const, object_ptr const fallback) const;

    /* behavior::associatively_readable */
    object_ptr get(object_ptr const elem) const;
    object_ptr get(object_ptr const elem, object_ptr const fallback) const;
    object_ptr get_entry(object_ptr const elem) const;
    native_bool contains(object_ptr const elem) const;

    transient_hash_set_ptr disjoin_in_place(object_ptr const elem);

    void assert_active() const;

    object base{ obj_type };
    value_type data;
    mutable native_hash hash{};
    native_bool active{ true };
  };
}
