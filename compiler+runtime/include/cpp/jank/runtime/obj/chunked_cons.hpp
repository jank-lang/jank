#pragma once

#include <jtl/option.hpp>

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using cons_ref = oref<struct cons>;
  using chunked_cons_ref = oref<struct chunked_cons>;

  struct chunked_cons : gc
  {
    static constexpr object_type obj_type{ object_type::chunked_cons };
    static constexpr native_bool pointer_free{ false };
    static constexpr native_bool is_sequential{ true };

    chunked_cons() = default;
    chunked_cons(chunked_cons &&) noexcept = default;
    chunked_cons(chunked_cons const &) = default;
    chunked_cons(object_ref head, object_ref tail);
    chunked_cons(object_ref meta, object_ref head, object_ref tail);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    void to_string(util::string_builder &buff) const;
    jtl::immutable_string to_string() const;
    jtl::immutable_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::metadatable */
    chunked_cons_ref with_meta(object_ref m) const;

    /* behavior::seqable */
    chunked_cons_ref seq() const;
    chunked_cons_ref fresh_seq() const;

    /* behavior::sequenceable */
    object_ref first() const;
    object_ref next() const;
    obj::cons_ref conj(object_ref head) const;

    /* behavior::sequenceable_in_place */
    chunked_cons_ref next_in_place();

    /* behavior::chunkable */
    object_ref chunked_first() const;
    object_ref chunked_next() const;

    object base{ obj_type };
    object_ref head{};
    object_ref tail{};
    jtl::option<object_ref> meta;
  };
}
