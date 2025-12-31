#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime
{
  native_big_decimal operator+(native_big_decimal const &l, native_big_integer const &r);
  native_big_decimal operator+(native_big_integer const &l, native_big_decimal const &r);
  native_big_decimal operator-(native_big_decimal const &l, native_big_integer const &r);
  native_big_decimal operator-(native_big_integer const &l, native_big_decimal const &r);
  native_big_decimal operator*(native_big_decimal const &l, native_big_integer const &r);
  native_big_decimal operator*(native_big_integer const &l, native_big_decimal const &r);
  native_big_decimal operator/(native_big_decimal const &l, native_big_integer const &r);
  native_big_decimal operator/(native_big_integer const &l, native_big_decimal const &r);
  bool operator==(native_big_decimal const &l, native_big_integer const &r);
  bool operator==(native_big_integer const &l, native_big_decimal const &r);
  bool operator!=(native_big_decimal const &l, native_big_integer const &r);
  bool operator!=(native_big_integer const &l, native_big_decimal const &r);
  bool operator<(native_big_decimal const &l, native_big_integer const &r);
  bool operator<(native_big_integer const &l, native_big_decimal const &r);
  bool operator<=(native_big_decimal const &l, native_big_integer const &r);
  bool operator<=(native_big_integer const &l, native_big_decimal const &r);
  bool operator>(native_big_decimal const &l, native_big_integer const &r);
  bool operator>(native_big_integer const &l, native_big_decimal const &r);
  bool operator>=(native_big_decimal const &l, native_big_integer const &r);
  bool operator>=(native_big_integer const &l, native_big_decimal const &r);
}

namespace jank::runtime::obj
{
  using big_decimal_ref = oref<struct big_decimal>;

  struct ratio;

  struct big_decimal
  {
    static constexpr object_type obj_type{ object_type::big_decimal };
    static constexpr bool pointer_free{ true };

    big_decimal() = default;
    big_decimal(big_decimal &&) noexcept = default;
    big_decimal(big_decimal const &) = default;

    explicit big_decimal(native_big_decimal const &);
    explicit big_decimal(native_big_decimal &&);
    explicit big_decimal(jtl::immutable_string const &);
    explicit big_decimal(native_big_integer const &);
    explicit big_decimal(ratio const &);

    /* behavior::object_like */
    bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    void to_string(jtl::string_builder &buff) const;
    jtl::immutable_string to_code_string() const;
    uhash to_hash() const;

    /* behavior::comparable */
    i64 compare(object const &) const;

    /* behavior::number_like */
    i64 to_integer() const;
    f64 to_real() const;

    static object_ref create(jtl::immutable_string const &);

    object base{ obj_type };
    native_big_decimal data{};
  };

  using jank::runtime::operator+;
  using jank::runtime::operator-;
  using jank::runtime::operator*;
  using jank::runtime::operator/;
  using jank::runtime::operator==;
  using jank::runtime::operator!=;
  using jank::runtime::operator<;
  using jank::runtime::operator<=;
  using jank::runtime::operator>;
  using jank::runtime::operator>=;
}
