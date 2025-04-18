#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/detail/type.hpp>

namespace jank::runtime::obj
{
  using transient_vector_ref = oref<struct transient_vector>;
  using persistent_vector_ref = oref<struct persistent_vector>;
  using persistent_vector_sequence_ref = oref<struct persistent_vector_sequence>;

  struct persistent_vector : gc
  {
    static constexpr object_type obj_type{ object_type::persistent_vector };
    static constexpr native_bool pointer_free{ false };
    static constexpr native_bool is_sequential{ true };

    using transient_type = transient_vector;
    using value_type = runtime::detail::native_persistent_vector;

    persistent_vector() = default;
    persistent_vector(persistent_vector &&) noexcept = default;
    persistent_vector(persistent_vector const &) = default;
    persistent_vector(value_type &&d);
    persistent_vector(value_type const &d);
    persistent_vector(jtl::option<object_ref> const &meta, value_type &&d);

    template <typename... Args>
    persistent_vector(std::in_place_t, Args &&...args)
      : data{ std::forward<Args>(args)... }
    {
    }

    template <typename... Args>
    persistent_vector(object_ref const meta, std::in_place_t, Args &&...args)
      : data{ std::forward<Args>(args)... }
      , meta{ meta }
    {
    }

    static persistent_vector_ref create(object_ref s);

    static persistent_vector_ref empty();

    /* behavior::object_like */
    native_bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    void to_string(util::string_builder &buff) const;
    jtl::immutable_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::comparable */
    i64 compare(object const &) const;

    /* behavior::comparable extended */
    i64 compare(persistent_vector const &) const;

    /* behavior::metadatable */
    persistent_vector_ref with_meta(object_ref m) const;

    /* behavior::seqable */
    persistent_vector_sequence_ref seq() const;
    persistent_vector_sequence_ref fresh_seq() const;

    /* behavior::countable */
    usize count() const;

    /* behavior::associatively_readable */
    object_ref get(object_ref key) const;
    object_ref get(object_ref key, object_ref fallback) const;
    object_ref get_entry(object_ref key) const;
    native_bool contains(object_ref key) const;

    /* behavior::conjable */
    persistent_vector_ref conj(object_ref head) const;

    /* behavior::stackable */
    object_ref peek() const;
    persistent_vector_ref pop() const;

    /* behavior::indexable */
    object_ref nth(object_ref index) const;
    object_ref nth(object_ref index, object_ref fallback) const;

    /* behavior::transientable */
    obj::transient_vector_ref to_transient() const;

    object base{ obj_type };
    value_type data;
    jtl::option<object_ref> meta;
    mutable native_hash hash{};
  };
}
