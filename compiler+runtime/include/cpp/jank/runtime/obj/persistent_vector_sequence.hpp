#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using cons_ref = jtl::oref<struct cons>;
  using persistent_vector_ref = jtl::oref<struct persistent_vector>;
  using persistent_vector_sequence_ref = jtl::oref<struct persistent_vector_sequence>;

  struct persistent_vector_sequence : gc
  {
    static constexpr object_type obj_type{ object_type::persistent_vector_sequence };
    static constexpr native_bool pointer_free{ false };
    static constexpr native_bool is_sequential{ true };

    persistent_vector_sequence() = default;
    persistent_vector_sequence(persistent_vector_sequence &&) noexcept = default;
    persistent_vector_sequence(persistent_vector_sequence const &) = default;
    persistent_vector_sequence(obj::persistent_vector_ref v);
    persistent_vector_sequence(obj::persistent_vector_ref v, size_t i);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    void to_string(util::string_builder &buff) const;
    jtl::immutable_string to_string() const;
    jtl::immutable_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::countable */
    size_t count() const;

    /* behavior::seqable */
    persistent_vector_sequence_ref seq();
    persistent_vector_sequence_ref fresh_seq() const;

    /* behavior::sequenceable */
    object_ptr first() const;
    persistent_vector_sequence_ref next() const;
    obj::cons_ref conj(object_ptr head);

    /* behavior::sequenceable_in_place */
    persistent_vector_sequence_ref next_in_place();

    object base{ obj_type };
    obj::persistent_vector_ref vec{};
    size_t index{};
  };
}
