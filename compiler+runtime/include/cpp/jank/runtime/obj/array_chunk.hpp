#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime
{
  template <>
  struct static_object<object_type::array_chunk> : gc
  {
    static constexpr native_bool pointer_free{ false };

    static_object() = default;
    static_object(native_vector<object_ptr> const &buffer);
    static_object(native_vector<object_ptr> const &buffer, size_t offset);
    static_object(native_vector<object_ptr> &&buffer, size_t offset);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    native_persistent_string to_string() const;
    void to_string(fmt::memory_buffer &buff) const;
    native_persistent_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::chunk_like */
    native_box<static_object> chunk_next() const;
    native_box<static_object> chunk_next_in_place();
    size_t count() const;
    object_ptr nth(object_ptr index) const;
    object_ptr nth(object_ptr index, object_ptr fallback) const;

    object base{ object_type::array_chunk };
    native_vector<object_ptr> buffer;
    size_t offset{};
  };

  namespace obj
  {
    using array_chunk = static_object<object_type::array_chunk>;
    using array_chunk_ptr = native_box<array_chunk>;
  }
}
