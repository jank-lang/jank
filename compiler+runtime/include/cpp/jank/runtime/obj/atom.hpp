#pragma once

#include <folly/Synchronized.h>

#include <jank/runtime/object.hpp>
#include <jank/runtime/obj/persistent_hash_map.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>

namespace jank::runtime::obj
{
  using atom_ref = oref<struct atom>;

  struct atom : object
  {
    static constexpr object_type obj_type{ object_type::atom };
    static constexpr object_behavior obj_behaviors{ object_behavior::none };
    static constexpr bool pointer_free{ false };

    atom();
    atom(object_ref const o);

    /* behavior::derefable */
    object_ref deref() const;

    /* Replaces the old value with the specified value. Returns the new value. */
    object_ref reset(object_ref const o);
    /* Same as reset, but returns a vector of the old value and the new value. */
    persistent_vector_ref reset_vals(object_ref const o);

    /* Atomically updates the value of the atom with the specified fn. Returns the new value. */
    object_ref swap(object_ref const fn);
    object_ref swap(object_ref const fn, object_ref const a1);
    object_ref swap(object_ref const fn, object_ref const a1, object_ref const a2);
    object_ref
    swap(object_ref const fn, object_ref const a1, object_ref const a2, object_ref const rest);

    /* Same as swap, but returns a vector of the old value and the new value. */
    persistent_vector_ref swap_vals(object_ref const fn);
    persistent_vector_ref swap_vals(object_ref const fn, object_ref const a1);
    persistent_vector_ref swap_vals(object_ref const fn, object_ref const a1, object_ref const a2);
    persistent_vector_ref
    swap_vals(object_ref const fn, object_ref const a1, object_ref const a2, object_ref const rest);

    object_ref compare_and_set(object_ref const old_val, object_ref const new_val);

    /* behavior::ref_like */
    void add_watch(object_ref const key, object_ref const fn);
    void remove_watch(object_ref const key);

    /*** XXX: Everything here is thread-safe. ***/

    /* We have to hold only a raw pointer here, since std::atomic doesn't
     * support more complex types. */
    std::atomic<object *> val{};
    /* Since watches is a `persistent_hash_map`, there in no guarantee in which
     * order watches are invoked. */
    folly::Synchronized<persistent_hash_map_ref> watches{};
  };
}
