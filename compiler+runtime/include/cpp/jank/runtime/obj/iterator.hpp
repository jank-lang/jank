#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/behavior/seqable.hpp>

namespace jank::runtime::obj
{
  using cons_ref = oref<struct cons>;
  using iterator_ref = oref<struct iterator>;
  using persistent_list_ref = oref<struct persistent_list>;

  /* TODO: Rename to iterator_sequence. */
  struct iterator : object
  {
    static constexpr object_type obj_type{ object_type::iterator };
    static constexpr object_behavior obj_behaviors{ object_behavior::seqable
                                                    | object_behavior::sequence_like
                                                    | object_behavior::sequence_like_in_place };
    static constexpr bool pointer_free{ false };
    static constexpr bool is_sequential{ true };

    iterator();
    iterator(object_ref const fn, object_ref const start);

    /* behavior::object_like */
    bool equal(object const &) const override;
    jtl::immutable_string to_string() const override;
    void to_string(jtl::string_builder &buff) const override;
    jtl::immutable_string to_code_string() const override;
    uhash to_hash() const override;

    /* behavior::seqable */
    object_ref seq() const override;
    object_ref fresh_seq() const override;

    /* behavior::sequence_like */
    object_ref first() const override;
    object_ref next() const override;

    /* behavior::conjable */
    obj::cons_ref conj(object_ref const head) const;

    /* behavior::collection_like */
    static persistent_list_ref empty();

    /* behavior::sequence_like_in_place */
    object_ref next_in_place() override;

    /*** XXX: Everything here is immutable after initialization. ***/
    /* TODO: Support chunking. */
    object_ref fn{};
    object_ref current{};
    object_ref previous{};

    /*** XXX: Everything here is thread-safe. ***/
    mutable std::atomic<iterator *> cached_next;
  };
}
