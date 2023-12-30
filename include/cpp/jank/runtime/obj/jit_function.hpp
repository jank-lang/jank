#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/behavior/callable.hpp>

namespace jank::runtime
{
  namespace obj
  {
    using persistent_array_map = static_object<object_type::persistent_array_map>;
    using persistent_array_map_ptr = native_box<persistent_array_map>;
  }

  template <>
  struct static_object<object_type::jit_function> : gc, behavior::callable
  {
    static constexpr bool pointer_free{ false };

    static_object() = default;
    static_object(static_object &&) = default;
    static_object(static_object const &) = default;
    static_object(object_ptr const fn, object_ptr const start);

    /* behavior::objectable */
    native_bool equal(object const &) const;
    native_persistent_string to_string();
    void to_string(fmt::memory_buffer &buff);
    native_integer to_hash() const;

    /* behavior::metadatable */
    object_ptr with_meta(object_ptr m);

    object base{ object_type::jit_function };
    behavior::callable_ptr data{};
    option<object_ptr> meta;
  };

  namespace obj
  {
    using jit_function = static_object<object_type::jit_function>;
    using jit_function_ptr = native_box<jit_function>;
  }
}
