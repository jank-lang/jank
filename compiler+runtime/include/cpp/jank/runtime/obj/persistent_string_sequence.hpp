#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using cons_ref = oref<struct cons>;
  using persistent_string_ref = oref<struct persistent_string>;
  using persistent_string_sequence_ref = oref<struct persistent_string_sequence>;

  struct persistent_string_sequence : gc
  {
    static constexpr object_type obj_type{ object_type::persistent_string_sequence };
    static constexpr bool pointer_free{ false };
    static constexpr bool is_sequential{ true };

    persistent_string_sequence() = default;
    persistent_string_sequence(persistent_string_sequence &&) noexcept = default;
    persistent_string_sequence(persistent_string_sequence const &) = default;
    persistent_string_sequence(obj::persistent_string_ref const s);
    persistent_string_sequence(obj::persistent_string_ref const s, usize const i);

    /* behavior::object_like */
    bool equal(object const &) const;
    void to_string(util::string_builder &buff) const;
    jtl::immutable_string to_string() const;
    jtl::immutable_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::countable */
    usize count() const;

    /* behavior::seqable */
    persistent_string_sequence_ref seq();
    persistent_string_sequence_ref fresh_seq() const;

    /* behavior::sequenceable */
    object_ref first() const;
    persistent_string_sequence_ref next() const;
    obj::cons_ref conj(object_ref head);

    /* behavior::sequenceable_in_place */
    persistent_string_sequence_ref next_in_place();

    object base{ obj_type };
    obj::persistent_string_ref str{};
    usize index{};
  };
}
