#pragma once

namespace jank::runtime
{
  template <>
  struct static_object<object_type::boolean> : gc
  {
    static constexpr bool pointer_free{ true };

    static native_box<static_object> true_const();
    static native_box<static_object> false_const();

    static_object() = default;
    static_object(static_object &&) = default;
    static_object(static_object const &) = default;
    static_object(native_bool const d);

    /* behavior::objectable */
    native_bool equal(object const &) const;
    native_string to_string() const;
    void to_string(fmt::memory_buffer &buff) const;
    native_integer to_hash() const;

    object base{ object_type::boolean };
    native_bool data{};
  };

  template <>
  struct static_object<object_type::integer> : gc
  {
    static constexpr bool pointer_free{ true };

    static_object() = default;
    static_object(static_object &&) = default;
    static_object(static_object const &) = default;
    static_object(native_integer const d);

    /* behavior::objectable */
    native_bool equal(object const &) const;
    native_string to_string() const;
    void to_string(fmt::memory_buffer &buff) const;
    native_integer to_hash() const;

    /* behavior::numberable */
    native_integer to_integer() const;
    native_real to_real() const;

    native_integer data{};
    object base{ object_type::integer };
  };

  template <>
  struct static_object<object_type::real> : gc
  {
    static constexpr bool pointer_free{ true };

    static_object() = default;
    static_object(static_object &&) = default;
    static_object(static_object const &) = default;
    static_object(native_real const d);

    /* behavior::objectable */
    native_bool equal(object const &) const;
    native_string to_string() const;
    void to_string(fmt::memory_buffer &buff) const;
    native_integer to_hash() const;

    /* behavior::numberable */
    native_integer to_integer() const;
    native_real to_real() const;

    native_real data{};
    object base{ object_type::real };
  };

  namespace obj
  {
    using boolean = static_object<object_type::boolean>;
    using boolean_ptr = native_box<boolean>;

    using integer = static_object<object_type::integer>;
    using integer_ptr = native_box<integer>;

    using real = static_object<object_type::real>;
    using real_ptr = native_box<real>;
  }
}
