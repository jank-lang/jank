#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/detail/type.hpp>

namespace jank::runtime::obj
{
  using transient_vector_ref = oref<struct transient_vector>;

  struct transient_vector : object
  {
    static constexpr object_type obj_type{ object_type::transient_vector };
    static constexpr object_behavior obj_behaviors{ object_behavior::call };
    static constexpr bool pointer_free{ false };

    using value_type = runtime::detail::native_transient_vector;
    using persistent_type_ref = oref<struct persistent_vector>;

    transient_vector();
    transient_vector(transient_vector &&) noexcept = default;
    transient_vector(transient_vector const &) = default;
    transient_vector(runtime::detail::native_persistent_vector const &d);
    transient_vector(runtime::detail::native_persistent_vector &&d);
    transient_vector(value_type &&d);

    static transient_vector_ref empty();

    /* behavior::countable */
    usize count() const;

    /* behavior::conjable_in_place */
    transient_vector_ref conj_in_place(object_ref const head);

    /* behavior::persistentable */
    persistent_type_ref to_persistent();

    /* behavior::callable */
    using object::call;
    object_ref call(object_ref const) const override;

    /* behavior::associatively_readable */
    object_ref get(object_ref const idx) const;
    object_ref get(object_ref const idx, object_ref const fallback) const;
    object_ref get_entry(object_ref const idx) const;
    bool contains(object_ref const elem) const;

    transient_vector_ref pop_in_place();

    void assert_active() const;

    /*** XXX: Everything here is not thread-safe, but not shared. ***/
    value_type data;
    bool active{ true };
  };
}
