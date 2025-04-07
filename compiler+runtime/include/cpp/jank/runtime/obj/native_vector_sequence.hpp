#pragma once

#include <jtl/option.hpp>

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using cons_ref = jtl::object_ref<struct cons>;
  using native_vector_sequence_ref = jtl::object_ref<struct native_vector_sequence>;

  struct native_vector_sequence : gc
  {
    static constexpr object_type obj_type{ object_type::native_vector_sequence };
    static constexpr native_bool pointer_free{ false };
    static constexpr native_bool is_sequential{ true };

    native_vector_sequence() = default;
    native_vector_sequence(native_vector_sequence &&) noexcept = default;
    native_vector_sequence(native_vector_sequence const &) = default;
    native_vector_sequence(native_vector<object_ptr> const &data, size_t index);
    native_vector_sequence(native_vector<object_ptr> &&data);
    native_vector_sequence(jtl::option<object_ptr> const &meta, native_vector<object_ptr> &&data);
    native_vector_sequence(native_vector<object_ptr> &&data, size_t index);

    /* behavior::object_like */
    native_bool equal(object const &o) const;
    void to_string(util::string_builder &buff) const;
    jtl::immutable_string to_string() const;
    jtl::immutable_string to_code_string() const;
    native_hash to_hash();

    /* behavior::seqable */
    native_vector_sequence_ref seq();
    native_vector_sequence_ref fresh_seq() const;

    /* behavior::countable */
    size_t count() const;

    /* behavior::sequence */
    object_ptr first() const;
    native_vector_sequence_ref next() const;
    obj::cons_ref conj(object_ptr head);

    /* behavior::sequenceable_in_place */
    native_vector_sequence_ref next_in_place();

    /* behavior::metadatable */
    native_vector_sequence_ref with_meta(object_ptr const m) const;

    object base{ obj_type };
    native_vector<object_ptr> data{};
    size_t index{};
    jtl::option<object_ptr> meta;
  };
}
