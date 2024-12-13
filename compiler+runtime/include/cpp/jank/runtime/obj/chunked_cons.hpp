#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/obj/cons.hpp>
#include <jank/option.hpp>

namespace jank::runtime::obj
{
  using chunked_cons_ptr = native_box<struct chunked_cons>;

  struct chunked_cons : gc
  {
    static constexpr object_type obj_type{ object_type::chunked_cons };
    static constexpr native_bool pointer_free{ false };
    static constexpr native_bool is_sequential{ true };

    chunked_cons() = default;
    chunked_cons(chunked_cons &&) noexcept = default;
    chunked_cons(chunked_cons const &) = default;
    chunked_cons(object_ptr head, object_ptr tail);
    chunked_cons(object_ptr meta, object_ptr head, object_ptr tail);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    void to_string(fmt::memory_buffer &buff) const;
    native_persistent_string to_string() const;
    native_persistent_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::metadatable */
    chunked_cons_ptr with_meta(object_ptr m) const;

    /* behavior::seqable */
    chunked_cons_ptr seq() const;
    chunked_cons_ptr fresh_seq() const;

    /* behavior::sequenceable */
    object_ptr first() const;
    object_ptr next() const;
    obj::cons_ptr conj(object_ptr head) const;

    /* behavior::sequenceable_in_place */
    chunked_cons_ptr next_in_place();

    /* behavior::chunkable */
    object_ptr chunked_first() const;
    object_ptr chunked_next() const;

    object base{ object_type::chunked_cons };
    object_ptr head{};
    object_ptr tail{};
    option<object_ptr> meta;
  };
}
