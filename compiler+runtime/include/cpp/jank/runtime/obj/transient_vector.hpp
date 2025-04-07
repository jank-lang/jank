#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/detail/type.hpp>

namespace jank::runtime::obj
{
  using transient_vector_ref = jtl::object_ref<struct transient_vector>;

  struct transient_vector : gc
  {
    static constexpr object_type obj_type{ object_type::transient_vector };
    static constexpr bool pointer_free{ false };

    using value_type = runtime::detail::native_transient_vector;
    using persistent_type_ref = jtl::object_ref<struct persistent_vector>;

    transient_vector() = default;
    transient_vector(transient_vector &&) noexcept = default;
    transient_vector(transient_vector const &) = default;
    transient_vector(runtime::detail::native_persistent_vector const &d);
    transient_vector(runtime::detail::native_persistent_vector &&d);
    transient_vector(value_type &&d);

    static transient_vector_ref empty();

    /* behavior::object_like */
    native_bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    void to_string(util::string_builder &buff) const;
    jtl::immutable_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::countable */
    size_t count() const;

    /* behavior::conjable_in_place */
    transient_vector_ref conj_in_place(object_ptr head);

    /* behavior::persistentable */
    persistent_type_ref to_persistent();

    /* behavior::callable */
    object_ptr call(object_ptr const) const;

    /* behavior::associatively_readable */
    object_ptr get(object_ptr const idx) const;
    object_ptr get(object_ptr const idx, object_ptr const fallback) const;
    object_ptr get_entry(object_ptr const idx) const;
    native_bool contains(object_ptr const elem) const;

    transient_vector_ref pop_in_place();

    void assert_active() const;

    object base{ obj_type };
    value_type data;
    mutable native_hash hash{};
    native_bool active{ true };
  };
}
