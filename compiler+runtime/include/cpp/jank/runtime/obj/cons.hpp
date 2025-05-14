#pragma once

#include <jtl/option.hpp>

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using cons_ref = oref<struct cons>;

  struct cons : gc
  {
    static constexpr object_type obj_type{ object_type::cons };
    static constexpr bool pointer_free{ false };
    static constexpr bool is_sequential{ true };

    cons() = default;
    cons(cons &&) noexcept = default;
    cons(cons const &) = default;
    cons(object_ref const head, object_ref const tail);

    /* behavior::object_like */
    bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    void to_string(util::string_builder &buff) const;
    jtl::immutable_string to_code_string() const;
    uhash to_hash() const;

    /* behavior::metadatable */
    cons_ref with_meta(object_ref m) const;

    /* behavior::seqable */
    cons_ref seq() const;
    cons_ref fresh_seq() const;

    /* behavior::sequenceable */
    object_ref first() const;
    object_ref next() const;

    /* behavior::conjable */
    cons_ref conj(object_ref head) const;

    object base{ obj_type };
    object_ref head{};
    object_ref tail{};
    mutable uhash hash{};
    jtl::option<object_ref> meta;
  };
}
