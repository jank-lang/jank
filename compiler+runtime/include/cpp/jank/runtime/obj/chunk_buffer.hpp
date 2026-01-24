#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using array_chunk_ref = oref<struct array_chunk>;
  using chunk_buffer_ref = oref<struct chunk_buffer>;

  struct chunk_buffer : object
  {
    static constexpr object_type obj_type{ object_type::chunk_buffer };
    static constexpr bool pointer_free{ false };

    chunk_buffer();
    chunk_buffer(usize capacity);
    chunk_buffer(object_ref const capacity);

    /* behavior::countable */
    usize count() const;

    void append(object_ref const o);
    obj::array_chunk_ref chunk();

    /*** XXX: Everything here is immutable after initialization. ***/
    usize capacity{};

    /*** XXX: Everything here is not thread-safe, but is not shared. ***/
    native_vector<object_ref> buffer;
  };
}
