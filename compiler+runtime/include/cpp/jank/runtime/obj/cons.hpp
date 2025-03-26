#pragma once

#include <jtl/option.hpp>

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using cons_ptr = native_box<struct cons>;

  struct cons : gc
  {
    static constexpr object_type obj_type{ object_type::cons };
    static constexpr native_bool pointer_free{ false };
    static constexpr native_bool is_sequential{ true };

    cons() = default;
    cons(cons &&) noexcept = default;
    cons(cons const &) = default;
    cons(object_ptr const head, object_ptr const tail);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    native_persistent_string to_string() const;
    void to_string(util::string_builder &buff) const;
    native_persistent_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::metadatable */
    cons_ptr with_meta(object_ptr m) const;

    /* behavior::seqable */
    cons_ptr seq() const;
    cons_ptr fresh_seq() const;

    /* behavior::sequenceable */
    object_ptr first() const;
    object_ptr next() const;

    /* behavior::sequenceable_in_place */
    cons_ptr next_in_place();

    /* behavior::conjable */
    cons_ptr conj(object_ptr head) const;

    object base{ obj_type };
    object_ptr head{};
    object_ptr tail{};
    mutable native_hash hash{};
    jtl::option<object_ptr> meta;
  };
}
