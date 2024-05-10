#pragma once

namespace jank::runtime
{
  template <>
  struct static_object<object_type::volatile_> : gc
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

    object base{ object_type::volatile_ };
    object_ptr val{};
  };

  namespace obj
  {
    using volatile_ = static_object<object_type::volatile_>;
    using volatile_ptr = native_box<volatile_>;
  }
}
