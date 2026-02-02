#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/behavior/seqable.hpp>

namespace jank::runtime::obj
{
  using cons_ref = oref<struct cons>;
  using iterator_ref = oref<struct iterator>;

  /* TODO: Rename to iterator_sequence. */
  struct iterator
  {
    static constexpr object_type obj_type{ object_type::iterator };
    static constexpr bool pointer_free{ false };
    static constexpr bool is_sequential{ true };

    iterator() = default;
    iterator(object_ref const fn, object_ref const start);

    /* behavior::object_like */
    bool equal(object const &) const;
    jtl::immutable_string to_string();
    void to_string(jtl::string_builder &buff);
    jtl::immutable_string to_code_string();
    uhash to_hash() const;

    /* behavior::seqable */
    iterator_ref seq();
    iterator_ref fresh_seq() const;

    /* behavior::sequenceable */
    object_ref first() const;
    iterator_ref next() const;
    obj::cons_ref conj(object_ref const head) const;

    /* behavior::sequenceable_in_place */
    iterator_ref next_in_place();

    /*** XXX: Everything here is immutable after initialization. ***/
    object base{ obj_type };
    /* TODO: Support chunking. */
    object_ref fn{};
    object_ref current{};
    object_ref previous{};

    /*** XXX: Everything here is thread-safe. ***/
    mutable std::atomic<iterator *> cached_next;
  };
}
