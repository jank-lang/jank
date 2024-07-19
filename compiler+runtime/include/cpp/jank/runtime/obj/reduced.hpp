#pragma once

namespace jank::runtime
{
  template <>
  struct static_object<object_type::reduced> : gc
  {
    static constexpr native_bool pointer_free{ false };

    static_object() = default;
    static_object(object_ptr o);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    native_persistent_string to_string() const;
    void to_string(fmt::memory_buffer &buff) const;
    native_hash to_hash() const;

    /* behavior::derefable */
    object_ptr deref() const;

    object base{ object_type::reduced };
    object_ptr val{};
  };

  namespace obj
  {
    using reduced = static_object<object_type::reduced>;
    using reduced_ptr = native_box<reduced>;
  }
}
