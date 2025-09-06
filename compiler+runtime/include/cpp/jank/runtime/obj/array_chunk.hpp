#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using array_chunk_ref = oref<struct array_chunk>;

  struct array_chunk : object
  {
    static constexpr object_type obj_type{ object_type::array_chunk };
    static constexpr bool pointer_free{ false };

    array_chunk();
    array_chunk(native_vector<object_ref> const &buffer);
    array_chunk(native_vector<object_ref> const &buffer, usize offset);
    array_chunk(native_vector<object_ref> &&buffer, usize offset);

    /* behavior::object_like */
    bool equal(object const &) const override;
    jtl::immutable_string to_string() const override;
    void to_string(jtl::string_builder &buff) const override;
    jtl::immutable_string to_code_string() const override;
    uhash to_hash() const override;

    /* behavior::chunk_like */
    array_chunk_ref chunk_next() const;
    array_chunk_ref chunk_next_in_place();
    usize count() const;
    object_ref nth(object_ref index) const;
    object_ref nth(object_ref index, object_ref fallback) const;

    object base{ obj_type };
    native_vector<object_ref> buffer;
    usize offset{};
  };
}
