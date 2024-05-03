#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime
{
  namespace obj
  {
    using array_chunk = static_object<object_type::array_chunk>;
    using array_chunk_ptr = native_box<array_chunk>;
  }

  template <>
  struct static_object<object_type::chunk_buffer> : gc
  {
    static constexpr native_bool pointer_free{ false };

    static_object() = default;
    static_object(size_t capacity);
    static_object(object_ptr capacity);

    /* behavior::objectable */
    native_bool equal(object const &) const;
    native_persistent_string to_string() const;
    void to_string(fmt::memory_buffer &buff) const;
    native_hash to_hash() const;

    /* behavior::countable */
    size_t count() const;

    void append(object_ptr o);
    obj::array_chunk_ptr chunk();

    object base{ object_type::chunk_buffer };
    native_vector<object_ptr> buffer;
    size_t capacity{};
  };

  namespace obj
  {
    using chunk_buffer = static_object<object_type::chunk_buffer>;
    using chunk_buffer_ptr = native_box<chunk_buffer>;
  }
}
