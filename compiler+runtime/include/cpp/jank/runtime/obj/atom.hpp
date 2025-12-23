#pragma once

#include <folly/Synchronized.h>

#include <jank/runtime/object.hpp>
#include <jank/runtime/obj/persistent_hash_map.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>

namespace jank::runtime::obj
{
  using atom_ref = oref<struct atom>;

  struct atom
  {
    static constexpr object_type obj_type{ object_type::atom };
    static constexpr bool pointer_free{ false };

    atom() = default;
    atom(object_ref const o);

    /* behavior::object_like */
    bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    void to_string(jtl::string_builder &buff) const;
    jtl::immutable_string to_code_string() const;
    uhash to_hash() const;

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

    object base{ obj_type };
    /* We have to hold only a raw pointer here, since std::atomic doesn't
     * support more complex types. However, that means we need to manually
     * handle reference counting for held values when swapping, resetting, etc. */
    std::atomic<object *> val{};
    /* Since watches is a 'persistent_hash_map", there in no guarantee in which
     * order watches are invoked. */
    folly::Synchronized<persistent_hash_map_ref> watches{};
  };
}
