#pragma once

#include <jtl/option.hpp>

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using cons_ref = oref<struct cons>;
  using chunked_cons_ref = oref<struct chunked_cons>;

  struct chunked_cons : object
  {
    static constexpr object_type obj_type{ object_type::chunked_cons };
    static constexpr object_behavior obj_behaviors{ object_behavior::none };
    static constexpr bool pointer_free{ false };
    static constexpr bool is_sequential{ true };

    chunked_cons();
    chunked_cons(chunked_cons &&) noexcept = default;
    chunked_cons(chunked_cons const &) = default;
    chunked_cons(object_ref const head, object_ref const tail);
    chunked_cons(object_ref const meta, object_ref const head, object_ref const tail);

    /* behavior::object_like */
    bool equal(object const &) const override;
    void to_string(jtl::string_builder &buff) const override;
    jtl::immutable_string to_string() const override;
    jtl::immutable_string to_code_string() const override;
    uhash to_hash() const override;

    /* behavior::metadatable */
    chunked_cons_ref with_meta(object_ref const m) const;
    object_ref get_meta() const;

    /* behavior::seqable */
    chunked_cons_ref seq() const;
    chunked_cons_ref fresh_seq() const;

    /* behavior::sequenceable */
    object_ref first() const;
    object_ref next() const;
    obj::cons_ref conj(object_ref const head) const;

    /* behavior::sequenceable_in_place */
    chunked_cons_ref next_in_place();

    /* behavior::chunkable */
    object_ref chunked_first() const;
    object_ref chunked_next() const;

    /*** XXX: Everything here is immutable after initialization. ***/
    object_ref head{};
    object_ref tail{};
    object_ref meta;
  };
}
