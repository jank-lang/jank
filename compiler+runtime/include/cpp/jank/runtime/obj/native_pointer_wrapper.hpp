#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime
{
  template <>
  struct static_object<object_type::native_pointer_wrapper> : gc
  {
    static constexpr native_bool pointer_free{ false };

    static_object() = default;
    static_object(static_object &&) = default;
    static_object(static_object const &) = default;
    static_object(void * const);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    native_persistent_string to_string() const;
    void to_string(fmt::memory_buffer &buff) const;
    native_persistent_string to_code_string() const;
    native_hash to_hash() const;

    template <typename T>
    T *as() const
    {
      return reinterpret_cast<T *>(data);
    }

    object base{ object_type::native_pointer_wrapper };

    void *data{};
  };

  namespace obj
  {
    using native_pointer_wrapper = static_object<object_type::native_pointer_wrapper>;
    using native_pointer_wrapper_ptr = native_box<native_pointer_wrapper>;
  }
}
