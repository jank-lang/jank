#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime
{
  f64 operator+(native_big_integer const &l, f64 const &r);
  f64 operator+(f64 const &l, native_big_integer const &r);
  f64 operator-(native_big_integer const &l, f64 const &r);
  f64 operator-(f64 const &l, native_big_integer const &r);
  f64 operator*(native_big_integer const &l, f64 const &r);
  f64 operator*(f64 const &l, native_big_integer const &r);
  f64 operator/(native_big_integer const &l, f64 const &r);
  f64 operator/(f64 const &l, native_big_integer const &r);
  bool operator==(native_big_integer const &l, f64 const &r);
  bool operator==(f64 const &l, native_big_integer const &r);
  bool operator!=(native_big_integer const &l, f64 const &r);
  bool operator!=(f64 const &l, native_big_integer const &r);
  bool operator<(native_big_integer const &l, f64 const &r);
  bool operator<(f64 const &l, native_big_integer const &r);
  bool operator<=(native_big_integer const &l, f64 const &r);
  bool operator<=(f64 const &l, native_big_integer const &r);
  bool operator>(native_big_integer const &l, f64 const &r);
  bool operator>(f64 const &l, native_big_integer const &r);
  bool operator>=(native_big_integer const &l, f64 const &r);
  bool operator>=(f64 const &l, native_big_integer const &r);
}

namespace jank::runtime::obj
{
  using big_integer_ref = oref<struct big_integer>;

  struct big_integer : gc
  {
    static constexpr object_type obj_type{ object_type::big_integer };
    static constexpr bool pointer_free{ true };

    big_integer() = default;
    big_integer(big_integer &&) noexcept = default;
    big_integer(big_integer const &) = default;

    explicit big_integer(native_big_integer const &);
    explicit big_integer(native_big_integer &&);
    explicit big_integer(i64);
    explicit big_integer(jtl::immutable_string const &);
    explicit big_integer(jtl::immutable_string const &, i64, bool);

    /* behavior::object_like */
    bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    void to_string(util::string_builder &buff) const;
    jtl::immutable_string to_code_string() const;
    uhash to_hash() const;

    /* behavior::comparable */
    i64 compare(object const &) const;

    /* behavior::comparable extended */
    i64 compare(big_integer const &) const;

    /* behavior::number_like */
    i64 to_integer() const;
    f64 to_real() const;

    static native_big_integer gcd(native_big_integer const &, native_big_integer const &);
    static i64 to_i64(native_big_integer const &);
    static f64 to_f64(native_big_integer const &);
    static uhash to_hash(native_big_integer const &);
    static object_ref create(jtl::immutable_string const &, i64, bool);

    void init(jtl::immutable_string const &);

    object base{ obj_type };
    native_big_integer data{};
  };

  /* For some reason, operators defined in jank::runtime namespace cannot be accessed from jank namespace.
   * We therefore added the following as a workaround. The root cause is not clear, likely due to boost cpp_int quirks.*/
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
