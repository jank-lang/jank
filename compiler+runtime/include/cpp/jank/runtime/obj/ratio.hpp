#pragma once

namespace jank::runtime
{
  namespace obj
  {
    struct ratio_data
    {
      ratio_data(native_integer const, native_integer const);
      ratio_data(ratio_data const &);
      native_real to_real() const;
      native_integer to_integer() const;
      native_integer numerator{};
      native_integer denominator{};
    };
  }

  template <>
  struct static_object<object_type::ratio> : gc
  {
    static constexpr native_bool pointer_free{ true };

    static_object() = default;
    static_object(static_object &&) = default;
    static_object(static_object const &) = default;
    static_object(obj::ratio_data const &);

    static object_ptr create(native_integer const, native_integer const);
    /* behavior::object_like */
    native_bool equal(object const &) const;
    native_persistent_string to_string() const;
    void to_string(fmt::memory_buffer &buff) const;
    native_persistent_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::comparable */
    native_integer compare(object const &) const;

    /* behavior::comparable extended */
    native_integer compare(static_object const &) const;

    /* behavior::number_like */
    native_integer to_integer() const;
    native_real to_real() const;

    object base{ object_type::ratio };
    obj::ratio_data data;
  };

  namespace obj
  {
    using ratio = static_object<object_type::ratio>;
    using ratio_ptr = native_box<ratio>;
  }

  object_ptr operator+(obj::ratio_data l, obj::ratio_data r);
  obj::ratio_ptr operator+(obj::integer_ptr l, obj::ratio_data r);
  obj::ratio_ptr operator+(obj::ratio_data l, obj::integer_ptr r);
  native_real operator+(obj::real_ptr l, obj::ratio_data r);
  native_real operator+(obj::ratio_data l, obj::real_ptr r);
  native_real operator+(obj::ratio_data l, native_real r);
  native_real operator+(native_real l, obj::ratio_data r);
  obj::ratio_ptr operator+(obj::ratio_data l, native_integer r);
  obj::ratio_ptr operator+(native_integer l, obj::ratio_data r);
  object_ptr operator-(obj::ratio_data l, obj::ratio_data r);
  obj::ratio_ptr operator-(obj::integer_ptr l, obj::ratio_data r);
  obj::ratio_ptr operator-(obj::ratio_data l, obj::integer_ptr r);
  native_real operator-(obj::real_ptr l, obj::ratio_data r);
  native_real operator-(obj::ratio_data l, obj::real_ptr r);
  native_real operator-(obj::ratio_data l, native_real r);
  native_real operator-(native_real l, obj::ratio_data r);
  obj::ratio_ptr operator-(obj::ratio_data l, native_integer r);
  obj::ratio_ptr operator-(native_integer l, obj::ratio_data r);
  object_ptr operator*(obj::ratio_data l, obj::ratio_data r);
  object_ptr operator*(obj::integer_ptr l, obj::ratio_data r);
  object_ptr operator*(obj::ratio_data l, obj::integer_ptr r);
  native_real operator*(obj::real_ptr l, obj::ratio_data r);
  native_real operator*(obj::ratio_data l, obj::real_ptr r);
  native_real operator*(obj::ratio_data l, native_real r);
  native_real operator*(native_real l, obj::ratio_data r);
  object_ptr operator*(obj::ratio_data l, native_integer r);
  object_ptr operator*(native_integer l, obj::ratio_data r);
  object_ptr operator/(obj::ratio_data l, obj::ratio_data r);
  object_ptr operator/(obj::integer_ptr l, obj::ratio_data r);
  obj::ratio_ptr operator/(obj::ratio_data l, obj::integer_ptr r);
  native_real operator/(obj::real_ptr l, obj::ratio_data r);
  native_real operator/(obj::ratio_data l, obj::real_ptr r);
  native_real operator/(obj::ratio_data l, native_real r);
  native_real operator/(native_real l, obj::ratio_data r);
  obj::ratio_ptr operator/(obj::ratio_data l, native_integer r);
  object_ptr operator/(native_integer l, obj::ratio_data r);
  native_bool operator==(obj::ratio_data l, obj::ratio_data r);
  native_bool operator==(obj::integer_ptr l, obj::ratio_data r);
  native_bool operator==(obj::ratio_data l, obj::integer_ptr r);
  native_bool operator==(obj::real_ptr l, obj::ratio_data r);
  native_bool operator==(obj::ratio_data l, obj::real_ptr r);
  native_bool operator==(obj::ratio_data l, native_real r);
  native_bool operator==(native_real l, obj::ratio_data r);
  native_bool operator==(obj::ratio_data l, native_integer r);
  native_bool operator==(native_integer l, obj::ratio_data r);
  native_bool operator<(obj::ratio_data l, obj::ratio_data r);
  native_bool operator<(obj::integer_ptr l, obj::ratio_data r);
  native_bool operator<(obj::ratio_data l, obj::integer_ptr r);
  native_bool operator<(obj::real_ptr l, obj::ratio_data r);
  native_bool operator<(obj::ratio_data l, obj::real_ptr r);
  native_bool operator<(obj::ratio_data l, native_real r);
  native_bool operator<(native_real l, obj::ratio_data r);
  native_bool operator<(obj::ratio_data l, native_integer r);
  native_bool operator<(native_integer l, obj::ratio_data r);
  native_bool operator<(native_bool l, obj::ratio_data r);
  native_bool operator<=(obj::ratio_data l, obj::ratio_data r);
  native_bool operator<=(obj::integer_ptr l, obj::ratio_data r);
  native_bool operator<=(obj::ratio_data l, obj::integer_ptr r);
  native_bool operator<=(obj::real_ptr l, obj::ratio_data r);
  native_bool operator<=(obj::ratio_data l, obj::real_ptr r);
  native_bool operator<=(obj::ratio_data l, native_real r);
  native_bool operator<=(native_real l, obj::ratio_data r);
  native_bool operator<=(obj::ratio_data l, native_integer r);
  native_bool operator<=(native_integer l, obj::ratio_data r);
  native_bool operator>(obj::ratio_data l, obj::ratio_data r);
  native_bool operator>(obj::integer_ptr l, obj::ratio_data r);
  native_bool operator>(obj::ratio_data l, obj::integer_ptr r);
  native_bool operator>(obj::real_ptr l, obj::ratio_data r);
  native_bool operator>(obj::ratio_data l, obj::real_ptr r);
  native_bool operator>(obj::ratio_data l, native_real r);
  native_bool operator>(native_real l, obj::ratio_data r);
  native_bool operator>(obj::ratio_data l, native_integer r);
  native_bool operator>(native_integer l, obj::ratio_data r);
  native_bool operator>(native_bool l, obj::ratio_data r);
  native_bool operator>=(obj::ratio_data l, obj::ratio_data r);
  native_bool operator>=(obj::integer_ptr l, obj::ratio_data r);
  native_bool operator>=(obj::ratio_data l, obj::integer_ptr r);
  native_bool operator>=(obj::real_ptr l, obj::ratio_data r);
  native_bool operator>=(obj::ratio_data l, obj::real_ptr r);
  native_bool operator>=(obj::ratio_data l, native_real r);
  native_bool operator>=(native_real l, obj::ratio_data r);
  native_bool operator>=(obj::ratio_data l, native_integer r);
  native_bool operator>=(native_integer l, obj::ratio_data r);
}
