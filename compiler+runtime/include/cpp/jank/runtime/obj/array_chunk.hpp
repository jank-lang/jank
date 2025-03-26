#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using array_chunk_ptr = native_box<struct array_chunk>;

  struct array_chunk : gc
  {
    static constexpr object_type obj_type{ object_type::array_chunk };
    static constexpr native_bool pointer_free{ false };

    array_chunk() = default;
    array_chunk(native_vector<object_ptr> const &buffer);
    array_chunk(native_vector<object_ptr> const &buffer, size_t offset);
    array_chunk(native_vector<object_ptr> &&buffer, size_t offset);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    void to_string(util::string_builder &buff) const;
    jtl::immutable_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::chunk_like */
    array_chunk_ptr chunk_next() const;
    array_chunk_ptr chunk_next_in_place();
    size_t count() const;
    object_ptr nth(object_ptr index) const;
    object_ptr nth(object_ptr index, object_ptr fallback) const;

    object base{ obj_type };
    native_vector<object_ptr> buffer;
    size_t offset{};
  };
}
