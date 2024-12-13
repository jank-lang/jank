#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>

namespace jank::runtime::obj
{
  using atom_ptr = native_box<struct atom>;

  struct atom : gc
  {
    static constexpr object_type obj_type{ object_type::atom };
    static constexpr native_bool pointer_free{ false };

    atom() = default;
    atom(object_ptr o);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    native_persistent_string to_string() const;
    void to_string(fmt::memory_buffer &buff) const;
    native_persistent_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::derefable */
    object_ptr deref() const;

    /* Replaces the old value with the specified value. Returns the new value. */
    object_ptr reset(object_ptr o);
    /* Same as reset, but returns a vector of the old value and the new value. */
    obj::persistent_vector_ptr reset_vals(object_ptr o);

    /* Atomically updates the value of the atom with the specified fn. Returns the new value. */
    object_ptr swap(object_ptr fn);
    object_ptr swap(object_ptr fn, object_ptr a1);
    object_ptr swap(object_ptr fn, object_ptr a1, object_ptr a2);
    object_ptr swap(object_ptr fn, object_ptr a1, object_ptr a2, object_ptr rest);

    /* Same as swap, but returns a vector of the old value and the new value. */
    obj::persistent_vector_ptr swap_vals(object_ptr fn);
    obj::persistent_vector_ptr swap_vals(object_ptr fn, object_ptr a1);
    obj::persistent_vector_ptr swap_vals(object_ptr fn, object_ptr a1, object_ptr a2);
    obj::persistent_vector_ptr
    swap_vals(object_ptr fn, object_ptr a1, object_ptr a2, object_ptr rest);

    object_ptr compare_and_set(object_ptr old_val, object_ptr new_val);

    object base{ object_type::atom };
    std::atomic<object *> val{};
  };
}
