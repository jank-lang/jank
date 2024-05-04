#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime
{
  template <>
  struct static_object<object_type::character> : gc
  {
    static constexpr native_bool pointer_free{ true };

    static_object() = default;
    static_object(static_object &&) = default;
    static_object(static_object const &) = default;
    static_object(native_char const &d);

    /* behavior::objectable */
    native_bool equal(object const &) const;
    native_persistent_string to_string() const;
    void to_string(fmt::memory_buffer &buff) const;
    native_hash to_hash() const;

    object base{ object_type::character };
    native_char data{};
  };

  namespace obj
  {
    using character = static_object<object_type::character>;
    using character_ptr = native_box<character>;
  }
}
