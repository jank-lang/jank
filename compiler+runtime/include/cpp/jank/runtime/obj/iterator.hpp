#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/behavior/seqable.hpp>

namespace jank::runtime::obj
{
  using cons_ref = jtl::oref<struct cons>;
  using iterator_ref = jtl::oref<struct iterator>;

  /* TODO: Rename to iterator_sequence. */
  struct iterator : gc
  {
    static constexpr object_type obj_type{ object_type::iterator };
    static constexpr native_bool pointer_free{ false };
    static constexpr native_bool is_sequential{ true };

    iterator() = default;
    iterator(iterator &&) noexcept = default;
    iterator(iterator const &) = default;
    iterator(object_ref const fn, object_ref const start);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    jtl::immutable_string to_string();
    void to_string(util::string_builder &buff);
    jtl::immutable_string to_code_string();
    native_hash to_hash() const;

    /* behavior::seqable */
    iterator_ref seq();
    iterator_ref fresh_seq() const;

    /* behavior::sequenceable */
    object_ref first() const;
    iterator_ref next() const;
    obj::cons_ref conj(object_ref head) const;

    /* behavior::sequenceable_in_place */
    iterator_ref next_in_place();

    object base{ obj_type };
    /* TODO: Support chunking. */
    object_ref fn{};
    object_ref current{};
    object_ref previous{};
    mutable iterator_ref cached_next{};
  };
}
