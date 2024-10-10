#pragma once

namespace jank::runtime
{
  template <>
  struct static_object<object_type::chunked_cons> : gc
  {
    static constexpr native_bool pointer_free{ false };

    static_object() = default;
    static_object(static_object &&) = default;
    static_object(static_object const &) = default;
    static_object(object_ptr head, object_ptr tail);
    static_object(object_ptr meta, object_ptr head, object_ptr tail);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    void to_string(fmt::memory_buffer &buff) const;
    native_persistent_string to_string() const;
    native_persistent_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::metadatable */
    native_box<static_object> with_meta(object_ptr m) const;

    /* behavior::seqable */
    native_box<static_object> seq() const;
    native_box<static_object> fresh_seq() const;

    /* behavior::sequenceable */
    object_ptr first() const;
    object_ptr next() const;
    obj::cons_ptr conj(object_ptr head) const;

    /* behavior::sequenceable_in_place */
    native_box<static_object> next_in_place();

    /* behavior::chunkable */
    object_ptr chunked_first() const;
    object_ptr chunked_next() const;

    object base{ object_type::chunked_cons };
    object_ptr head{};
    object_ptr tail{};
    option<object_ptr> meta;
  };

  namespace obj
  {
    using chunked_cons = static_object<object_type::chunked_cons>;
    using chunked_cons_ptr = native_box<chunked_cons>;
  }
}
