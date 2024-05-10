#pragma once

namespace jank::runtime
{
  template <>
  struct static_object<object_type::atom> : gc
  {
    static constexpr native_bool pointer_free{ false };

    static_object() = default;
    static_object(object_ptr o);

    /* behavior::objectable */
    native_bool equal(object const &) const;
    native_persistent_string to_string() const;
    void to_string(fmt::memory_buffer &buff) const;
    native_hash to_hash() const;

    /* behavior::derefable */
    object_ptr deref() const;

    object_ptr reset(object_ptr o);
    obj::persistent_vector_ptr reset_vals(object_ptr o);

    object_ptr swap(object_ptr fn);
    object_ptr swap(object_ptr fn, object_ptr a1);
    object_ptr swap(object_ptr fn, object_ptr a1, object_ptr a2);
    object_ptr swap(object_ptr fn, object_ptr a1, object_ptr a2, object_ptr rest);

    obj::persistent_vector_ptr swap_vals(object_ptr fn);
    obj::persistent_vector_ptr swap_vals(object_ptr fn, object_ptr a1);
    obj::persistent_vector_ptr swap_vals(object_ptr fn, object_ptr a1, object_ptr a2);
    obj::persistent_vector_ptr
    swap_vals(object_ptr fn, object_ptr a1, object_ptr a2, object_ptr rest);

    object_ptr compare_and_set(object_ptr old_val, object_ptr new_val);

    object base{ object_type::atom };
    std::atomic<object *> val{};
  };

  namespace obj
  {
    using atom = static_object<object_type::atom>;
    using atom_ptr = native_box<atom>;
  }
}
