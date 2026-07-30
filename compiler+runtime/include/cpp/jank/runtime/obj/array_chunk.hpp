#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using array_chunk_ref = oref<struct array_chunk>;

  struct array_chunk : object
  {
    static constexpr object_type obj_type{ object_type::array_chunk };
    static constexpr object_behavior obj_behaviors{ object_behavior::indexable };
    static constexpr bool pointer_free{ false };

    array_chunk();
    array_chunk(native_vector<object_ref> const &buffer);
    array_chunk(native_vector<object_ref> const &buffer, usize offset);
    array_chunk(native_vector<object_ref> &&buffer, usize offset);

    /* behavior::chunk_like */
    array_chunk_ref chunk_next() const;
    array_chunk_ref chunk_next_in_place();
    usize count() const;

    /* behavior::indexable */
    object_ref nth(object_ref const index) const override;
    object_ref nth(object_ref const index, object_ref const fallback) const override;

    /*** XXX: Everything here is immutable after initialization. ***/
    native_vector<object_ref> buffer;
    usize offset{};
  };
}
