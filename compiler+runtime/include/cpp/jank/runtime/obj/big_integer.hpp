#pragma once

#include <jank/runtime/object.hpp>
#include <boost/multiprecision/cpp_int.hpp>

namespace jank::runtime
{
  native_real operator+(native_big_integer const &l, native_real const &r);
  native_real operator+(native_real const &l, native_big_integer const &r);
  native_real operator-(native_big_integer const &l, native_real const &r);
  native_real operator-(native_real const &l, native_big_integer const &r);
  native_real operator*(native_big_integer const &l, native_real const &r);
  native_real operator*(native_real const &l, native_big_integer const &r);
  native_real operator/(native_big_integer const &l, native_real const &r);
  native_real operator/(native_real const &l, native_big_integer const &r);
  native_bool operator==(native_big_integer const &l, native_real const &r);
  native_bool operator==(native_real const &l, native_big_integer const &r);
  native_bool operator!=(native_big_integer const &l, native_real const &r);
  native_bool operator!=(native_real const &l, native_big_integer const &r);
  native_bool operator<(native_big_integer const &l, native_real const &r);
  native_bool operator<(native_real const &l, native_big_integer const &r);
  native_bool operator<=(native_big_integer const &l, native_real const &r);
  native_bool operator<=(native_real const &l, native_big_integer const &r);
  native_bool operator>(native_big_integer const &l, native_real const &r);
  native_bool operator>(native_real const &l, native_big_integer const &r);
  native_bool operator>=(native_big_integer const &l, native_real const &r);
  native_bool operator>=(native_real const &l, native_big_integer const &r);
}

namespace jank::runtime::obj
{
  using big_integer_ptr = native_box<struct big_integer>;
  using native_big_integer = boost::multiprecision::cpp_int;

  struct big_integer : gc
  {
    static constexpr object_type obj_type{ object_type::big_integer };
    static constexpr native_bool pointer_free{ false };

    big_integer();
    big_integer(big_integer &&) noexcept = default;
    big_integer(big_integer const &) = default;
    big_integer(native_big_integer const &);
    big_integer(native_big_integer &&);
    big_integer(native_integer);
    explicit big_integer(native_persistent_string_view const &);
    explicit big_integer(native_persistent_string_view const &, native_integer, native_bool);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    native_persistent_string to_string() const;
    void to_string(util::string_builder &buff) const;
    native_persistent_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::comparable */
    native_integer compare(object const &) const;

    /* behavior::comparable extended */
    native_integer compare(big_integer const &) const;

    /* behavior::number_like */
    native_integer to_integer() const;
    native_real to_real() const;
    static native_big_integer gcd(native_big_integer const &, native_big_integer const &);
    static native_integer to_native_integer(native_big_integer const &);
    static native_real to_native_real(native_big_integer const &);
    static native_hash to_hash(native_big_integer const &);
    static object_ptr create(native_persistent_string_view const &, native_integer, native_bool);
    void init(native_persistent_string_view const &);
    object base{ obj_type };
    native_big_integer data;
  };

  using jank::runtime::operator+;
  using jank::runtime::operator+;
  using jank::runtime::operator-;
  using jank::runtime::operator-;
  using jank::runtime::operator*;
  using jank::runtime::operator*;
  using jank::runtime::operator/;
  using jank::runtime::operator/;
  using jank::runtime::operator==;
  using jank::runtime::operator==;
  using jank::runtime::operator!=;
  using jank::runtime::operator!=;
  using jank::runtime::operator<;
  using jank::runtime::operator<;
  using jank::runtime::operator<=;
  using jank::runtime::operator<=;
  using jank::runtime::operator>;
  using jank::runtime::operator>;
  using jank::runtime::operator>=;
  using jank::runtime::operator>=;
}

namespace std
{
  template <>
  struct hash<jank::runtime::obj::big_integer_ptr>
  {
    size_t operator()(jank::runtime::obj::big_integer_ptr const o) const noexcept
    {
      return o->to_hash();
    }
  };

  template <>
  struct hash<jank::runtime::obj::big_integer>
  {
    size_t operator()(jank::runtime::obj::big_integer const &o) const noexcept
    {
      std::hash<boost::multiprecision::cpp_int> hasher;
      return hasher(o.data);
    }
  };
} // namespace std
