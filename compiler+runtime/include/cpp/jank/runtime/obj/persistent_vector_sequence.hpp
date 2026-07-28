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
    static constexpr object_behavior obj_behaviors{ object_behavior::seqable
                                                    | object_behavior::fresh_seqable
                                                    | object_behavior::sequence_like
                                                    | object_behavior::sequence_like_in_place };
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
    object_ref seq() const override;

    /* behavior::fresh_seqable */
    object_ref fresh_seq() const override;

    /* behavior::sequence_like */
    object_ref first() const override;
    object_ref next() const override;

    /* behavior::conjable */
    obj::cons_ref conj(object_ref const head);

    /* behavior::sequence_like_in_place */
    object_ref next_in_place() override;

    /*** XXX: Everything here is immutable after initialization. ***/
    obj::persistent_vector_ref vec{};
    usize index{};
  };
}
