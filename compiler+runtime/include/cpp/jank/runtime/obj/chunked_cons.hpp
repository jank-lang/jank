#pragma once

#include <jtl/option.hpp>

#include <jank/runtime/object.hpp>
#include <jank/runtime/lazy_meta.hpp>

namespace jank::runtime::obj
{
  using cons_ref = oref<struct cons>;
  using chunked_cons_ref = oref<struct chunked_cons>;

  struct chunked_cons : object
  {
    static constexpr object_type obj_type{ object_type::chunked_cons };
    static constexpr object_behavior obj_behaviors{ object_behavior::seqable
                                                    | object_behavior::fresh_seqable
                                                    | object_behavior::sequence_like
                                                    | object_behavior::sequence_like_in_place };
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
    void set_meta(object_ref const o);

    /* behavior::seqable */
    object_ref seq() const override;

    /* behavior::fresh_seqable */
    object_ref fresh_seq() const override;

    /* behavior::sequence_like */
    object_ref first() const override;
    object_ref next() const override;

    /* behavior::conjable */
    obj::cons_ref conj(object_ref const head) const;

    /* behavior::sequence_like_in_place */
    object_ref next_in_place() override;

    /* behavior::chunkable */
    object_ref chunked_first() const;
    object_ref chunked_next() const;

    /*** XXX: Everything here is immutable after initialization. ***/
    object_ref head{};
    object_ref tail{};

    /*** XXX: Everything here is thread-safe. ***/
  private:
    lazy_meta meta;
  };
}
