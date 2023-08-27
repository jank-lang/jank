#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime
{
  /* TODO: Seqable. */
  template <>
  struct static_object<object_type::string> : gc
  {
    static constexpr bool pointer_free{ true };

    static_object() = default;
    static_object(static_object &&) = default;
    static_object(static_object const &) = default;
    static_object(object &&base);
    static_object(native_string const &d);
    static_object(native_string &&d);

    /* behavior::objectable */
    native_bool equal(object const &) const;
    native_string const& to_string() const;
    void to_string(fmt::memory_buffer &buff) const;
    native_integer to_hash() const;

    /* behavior::countable */
    size_t count() const;

    object base{ object_type::string };
    native_string data;
  };

  namespace obj
  {
    using string = static_object<object_type::string>;
    using string_ptr = native_box<string>;
  }
}
