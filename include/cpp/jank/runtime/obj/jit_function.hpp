#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/behavior/callable.hpp>

namespace jank::runtime
{
  namespace obj
  {
    using map = static_object<object_type::map>;
    using map_ptr = native_box<map>;
  }

  template <>
  struct static_object<object_type::jit_function> : gc, behavior::callable
  {
    static constexpr bool pointer_free{ false };

    static_object() = default;
    static_object(static_object &&) = default;
    static_object(static_object const &) = default;
    static_object(object &&base);
    static_object(object_ptr const fn, object_ptr const start);

    /* behavior::objectable */
    native_bool equal(object const &) const;
    native_string to_string();
    void to_string(fmt::memory_buffer &buff);
    native_integer to_hash() const;

    /* behavior::metadatable */
    object_ptr with_meta(object_ptr m);

    /* TODO: Doesn't have an offset of 0. */
    object base{ object_type::jit_function };
    behavior::callable_ptr data{};
    option<obj::map_ptr> meta;
  };

  namespace obj
  {
    using jit_function = static_object<object_type::jit_function>;
    using jit_function_ptr = native_box<jit_function>;
  }
}
