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
    chunk_buffer(object_ref capacity);

    /* behavior::object_like */
    bool equal(object const &) const override;
    jtl::immutable_string to_string() const override;
    void to_string(jtl::string_builder &buff) const override;
    jtl::immutable_string to_code_string() const override;
    uhash to_hash() const override;

    /* behavior::countable */
    usize count() const;

    void append(object_ref o);
    obj::array_chunk_ref chunk();

    native_vector<object_ref> buffer;
    usize capacity{};
  };
}
