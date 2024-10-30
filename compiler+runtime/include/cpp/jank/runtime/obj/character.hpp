#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime
{
  template <>
  struct static_object<object_type::character> : gc
  {
    static constexpr native_bool pointer_free{ false };

    static_object() = default;
    static_object(static_object &&) = default;
    static_object(static_object const &) = default;
    static_object(native_persistent_string const &);
    static_object(char);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    native_persistent_string to_string() const;
    void to_string(fmt::memory_buffer &buff) const;
    native_persistent_string to_code_string() const;
    native_hash to_hash() const;

    object base{ object_type::character };

    /* Holds the literal form of the character as it's written eg. "\\tab" */
    native_persistent_string data;
  };

  namespace obj
  {
    using character = static_object<object_type::character>;
    using character_ptr = native_box<character>;
  }
}
