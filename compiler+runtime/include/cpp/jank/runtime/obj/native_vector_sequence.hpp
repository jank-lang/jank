#pragma once

#include <jtl/option.hpp>

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using cons_ref = oref<struct cons>;
  using native_vector_sequence_ref = oref<struct native_vector_sequence>;

  struct native_vector_sequence : gc
  {
    static constexpr object_type obj_type{ object_type::native_vector_sequence };
    static constexpr bool pointer_free{ false };
    static constexpr bool is_sequential{ true };

    native_vector_sequence() = default;
    native_vector_sequence(native_vector_sequence &&) noexcept = default;
    native_vector_sequence(native_vector_sequence const &) = default;
    native_vector_sequence(native_vector<object_ref> const &data, usize index);
    native_vector_sequence(native_vector<object_ref> &&data);
    native_vector_sequence(jtl::option<object_ref> const &meta, native_vector<object_ref> &&data);
    native_vector_sequence(native_vector<object_ref> &&data, usize index);

    /* behavior::object_like */
    bool equal(object const &o) const;
    void to_string(util::string_builder &buff) const;
    jtl::immutable_string to_string() const;
    jtl::immutable_string to_code_string() const;
    uhash to_hash();

    /* behavior::seqable */
    native_vector_sequence_ref seq();
    native_vector_sequence_ref fresh_seq() const;

    /* behavior::countable */
    usize count() const;

    /* behavior::sequence */
    object_ref first() const;
    native_vector_sequence_ref next() const;
    obj::cons_ref conj(object_ref head);

    /* behavior::sequenceable_in_place */
    native_vector_sequence_ref next_in_place();

    /* behavior::metadatable */
    native_vector_sequence_ref with_meta(object_ref const m) const;

    object base{ obj_type };
    native_vector<object_ref> data{};
    usize index{};
    jtl::option<object_ref> meta;
  };
}
