#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using cons_ref = oref<struct cons>;
  using persistent_vector_ref = oref<struct persistent_vector>;
  using persistent_vector_sequence_ref = oref<struct persistent_vector_sequence>;

  struct persistent_vector_sequence : object
  {
    static constexpr object_type obj_type{ object_type::persistent_vector_sequence };
    static constexpr bool pointer_free{ false };
    static constexpr bool is_sequential{ true };

    persistent_vector_sequence();
    persistent_vector_sequence(persistent_vector_sequence &&) noexcept = default;
    persistent_vector_sequence(persistent_vector_sequence const &) = default;
    persistent_vector_sequence(obj::persistent_vector_ref const v);
    persistent_vector_sequence(obj::persistent_vector_ref const v, usize i);

    /* behavior::object_like */
    bool equal(object const &) const override;
    void to_string(jtl::string_builder &buff) const override;
    jtl::immutable_string to_string() const override;
    jtl::immutable_string to_code_string() const override;
    uhash to_hash() const override;

    /* behavior::countable */
    usize count() const;

    /* behavior::seqable */
    persistent_vector_sequence_ref seq();
    persistent_vector_sequence_ref fresh_seq() const;

    /* behavior::sequenceable */
    object_ref first() const;
    persistent_vector_sequence_ref next() const;
    obj::cons_ref conj(object_ref const head);

    /* behavior::sequenceable_in_place */
    persistent_vector_sequence_ref next_in_place();

    /*** XXX: Everything here is immutable after initialization. ***/
    obj::persistent_vector_ref vec{};
    usize index{};
  };
}
