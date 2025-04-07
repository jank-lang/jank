#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using array_chunk_ref = jtl::object_ref<struct array_chunk>;
  using chunk_buffer_ref = jtl::object_ref<struct chunk_buffer>;

  struct chunk_buffer : gc
  {
    static constexpr object_type obj_type{ object_type::chunk_buffer };
    static constexpr native_bool pointer_free{ false };

    chunk_buffer() = default;
    chunk_buffer(size_t capacity);
    chunk_buffer(object_ptr capacity);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    void to_string(util::string_builder &buff) const;
    jtl::immutable_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::countable */
    size_t count() const;

    void append(object_ptr o);
    obj::array_chunk_ref chunk();

    object base{ obj_type };
    native_vector<object_ptr> buffer;
    size_t capacity{};
  };
}
