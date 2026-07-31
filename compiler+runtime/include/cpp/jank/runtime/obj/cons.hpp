#pragma once

#include <jtl/option.hpp>

#include <jank/runtime/object.hpp>
#include <jank/runtime/lazy_meta.hpp>

namespace jank::runtime::obj
{
  using cons_ref = oref<struct cons>;

  struct cons : object
  {
    static constexpr object_type obj_type{ object_type::cons };
    static constexpr object_behavior obj_behaviors{ object_behavior::seqable
                                                    | object_behavior::sequence_like };
    static constexpr bool pointer_free{ false };
    static constexpr bool is_sequential{ true };

    cons();
    cons(object_ref const head, object_ref const tail);

    /* behavior::object_like */
    bool equal(object const &) const override;
    jtl::immutable_string to_string() const override;
    void to_string(jtl::string_builder &buff) const override;
    jtl::immutable_string to_code_string() const override;
    uhash to_hash() const override;

    /* behavior::metadatable */
    cons_ref with_meta(object_ref const m) const;
    object_ref get_meta() const;
    void set_meta(object_ref const o);

    /* behavior::seqable */
    object_ref seq() const override;
    object_ref fresh_seq() const override;

    /* behavior::sequence_like */
    object_ref first() const override;
    object_ref next() const override;

    /* behavior::conjable */
    cons_ref conj(object_ref const head) const;

    /*** XXX: Everything here is immutable after initialization. ***/
    object_ref head{};
    object_ref tail{};

    /*** XXX: Everything here is thread-safe. ***/
    lazy_meta meta;
    mutable std::atomic<uhash> hash{};
  };
}
