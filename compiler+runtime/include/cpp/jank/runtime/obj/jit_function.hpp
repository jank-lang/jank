#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/behavior/callable.hpp>

namespace jank::runtime
{
  template <>
  struct static_object<object_type::jit_function>
    : gc
    , behavior::callable
  {
    static constexpr native_bool pointer_free{ false };

    static_object() = default;
    static_object(static_object &&) = default;
    static_object(static_object const &) = default;
    static_object(object_ptr meta);

    /* behavior::objectable */
    native_bool equal(object const &) const;
    native_persistent_string to_string();
    void to_string(fmt::memory_buffer &buff);
    native_hash to_hash() const;

    /* behavior::metadatable */
    native_box<static_object> with_meta(object_ptr m);

    /* behavior::callable */
    object_ptr this_object_ptr() final;

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
