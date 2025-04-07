#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/detail/type.hpp>

namespace jank::runtime::obj
{
  using transient_vector_ref = jtl::object_ref<struct transient_vector>;
  using persistent_vector_ref = jtl::object_ref<struct persistent_vector>;
  using persistent_vector_sequence_ref = jtl::object_ref<struct persistent_vector_sequence>;

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
    persistent_vector(jtl::option<object_ptr> const &meta, value_type &&d);

    template <typename... Args>
    persistent_vector(std::in_place_t, Args &&...args)
      : data{ std::forward<Args>(args)... }
    {
    }

    template <typename... Args>
    persistent_vector(object_ptr const meta, std::in_place_t, Args &&...args)
      : data{ std::forward<Args>(args)... }
      , meta{ meta }
    {
    }

    static persistent_vector_ref create(object_ptr s);

    static persistent_vector_ref empty();

    /* behavior::object_like */
    native_bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    void to_string(util::string_builder &buff) const;
    jtl::immutable_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::comparable */
    native_integer compare(object const &) const;

    /* behavior::comparable extended */
    native_integer compare(persistent_vector const &) const;

    /* behavior::metadatable */
    persistent_vector_ref with_meta(object_ptr m) const;

    /* behavior::seqable */
    persistent_vector_sequence_ref seq() const;
    persistent_vector_sequence_ref fresh_seq() const;

    /* behavior::countable */
    size_t count() const;

    /* behavior::associatively_readable */
    object_ptr get(object_ptr key) const;
    object_ptr get(object_ptr key, object_ptr fallback) const;
    object_ptr get_entry(object_ptr key) const;
    native_bool contains(object_ptr key) const;

    /* behavior::conjable */
    persistent_vector_ref conj(object_ptr head) const;

    /* behavior::stackable */
    object_ptr peek() const;
    persistent_vector_ref pop() const;

    /* behavior::indexable */
    object_ptr nth(object_ptr index) const;
    object_ptr nth(object_ptr index, object_ptr fallback) const;

    /* behavior::transientable */
    obj::transient_vector_ref to_transient() const;

    object base{ obj_type };
    value_type data;
    jtl::option<object_ptr> meta;
    mutable native_hash hash{};
  };
}
