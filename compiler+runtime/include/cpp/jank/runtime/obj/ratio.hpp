#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/obj/big_integer.hpp>

namespace jank::runtime::obj
{
  struct ratio_data
  {
    ratio_data(i64 const, i64 const);
    ratio_data(native_big_integer const &, native_big_integer const &);
    ratio_data(big_integer const &, big_integer const &);
    ratio_data(object_ref, object_ref);
    ratio_data(ratio_data const &) = default;

    f64 to_real() const;
    i64 to_integer() const;

    native_big_integer numerator{};
    native_big_integer denominator{};
  };

  using integer_ref = oref<struct integer>;
  using real_ref = oref<struct real>;
  using ratio_ref = oref<struct ratio>;

  struct ratio : gc
  {
    static constexpr object_type obj_type{ object_type::ratio };
    static constexpr bool pointer_free{ true };

    ratio(ratio &&) noexcept = default;
    ratio(ratio const &) = default;
    ratio(ratio_data const &);

    static object_ref create(i64 const, i64 const);
    static object_ref create(native_big_integer const &, native_big_integer const &);

    /* behavior::object_like */
    bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    void to_string(util::string_builder &buff) const;
    jtl::immutable_string to_code_string() const;
    uhash to_hash() const;

    /* behavior::comparable */
    i64 compare(object const &) const;

    /* behavior::comparable extended */
    i64 compare(ratio const &) const;

    /* behavior::number_like */
    i64 to_integer() const;
    f64 to_real() const;

    object base{ obj_type };
    ratio_data data;
  };

  object_ref operator+(ratio_data const &l, ratio_data const &r);
  ratio_ref operator+(integer_ref l, ratio_data const &r);
  ratio_ref operator+(ratio_data const &l, integer_ref r);
  f64 operator+(real_ref l, ratio_data const &r);
  f64 operator+(ratio_data const &l, real_ref r);
  f64 operator+(ratio_data const &l, f64 r);
  f64 operator+(f64 l, ratio_data const &r);
  ratio_ref operator+(ratio_data const &l, i64 r);
  ratio_ref operator+(i64 l, ratio_data const &r);
  object_ref operator-(ratio_data const &l, ratio_data const &r);
  ratio_ref operator-(integer_ref l, ratio_data const &r);
  ratio_ref operator-(ratio_data const &l, integer_ref r);
  f64 operator-(real_ref l, ratio_data const &r);
  f64 operator-(ratio_data const &l, real_ref r);
  f64 operator-(ratio_data const &l, f64 r);
  f64 operator-(f64 l, ratio_data const &r);
  ratio_ref operator-(ratio_data const &l, i64 r);
  ratio_ref operator-(i64 l, ratio_data const &r);
  object_ref operator*(ratio_data const &l, ratio_data const &r);
  object_ref operator*(integer_ref l, ratio_data const &r);
  object_ref operator*(ratio_data const &l, integer_ref r);
  f64 operator*(real_ref l, ratio_data const &r);
  f64 operator*(ratio_data const &l, real_ref r);
  f64 operator*(ratio_data const &l, f64 r);
  f64 operator*(f64 l, ratio_data const &r);
  object_ref operator*(ratio_data const &l, i64 r);
  object_ref operator*(i64 l, ratio_data const &r);
  object_ref operator/(ratio_data const &l, ratio_data const &r);
  object_ref operator/(integer_ref l, ratio_data const &r);
  ratio_ref operator/(ratio_data const &l, integer_ref r);
  f64 operator/(real_ref l, ratio_data const &r);
  f64 operator/(ratio_data const &l, real_ref r);
  f64 operator/(ratio_data const &l, f64 r);
  f64 operator/(f64 l, ratio_data const &r);
  ratio_ref operator/(ratio_data const &l, i64 r);
  object_ref operator/(i64 l, ratio_data const &r);
  bool operator==(ratio_data const &l, ratio_data const &r);
  bool operator==(integer_ref l, ratio_data const &r);
  bool operator==(ratio_data const &l, integer_ref r);
  bool operator==(real_ref l, ratio_data const &r);
  bool operator==(ratio_data const &l, real_ref r);
  bool operator==(ratio_data const &l, f64 r);
  bool operator==(f64 l, ratio_data const &r);
  bool operator==(ratio_data const &l, i64 r);
  bool operator==(i64 l, ratio_data const &r);
  bool operator<(ratio_data const &l, ratio_data const &r);
  bool operator<(integer_ref l, ratio_data const &r);
  bool operator<(ratio_data const &l, integer_ref r);
  bool operator<(real_ref l, ratio_data const &r);
  bool operator<(ratio_data const &l, real_ref r);
  bool operator<(ratio_data const &l, f64 r);
  bool operator<(f64 l, ratio_data const &r);
  bool operator<(ratio_data const &l, i64 r);
  bool operator<(i64 l, ratio_data const &r);
  bool operator<(bool l, ratio_data const &r);
  bool operator<(ratio_data const &l, bool r);
  bool operator<=(ratio_data const &l, ratio_data const &r);
  bool operator<=(integer_ref l, ratio_data const &r);
  bool operator<=(ratio_data const &l, integer_ref r);
  bool operator<=(real_ref l, ratio_data const &r);
  bool operator<=(ratio_data const &l, real_ref r);
  bool operator<=(ratio_data const &l, f64 r);
  bool operator<=(f64 l, ratio_data const &r);
  bool operator<=(ratio_data const &l, i64 r);
  bool operator<=(i64 l, ratio_data const &r);
  bool operator>(ratio_data const &l, ratio_data const &r);
  bool operator>(integer_ref l, ratio_data const &r);
  bool operator>(ratio_data const &l, integer_ref r);
  bool operator>(real_ref l, ratio_data const &r);
  bool operator>(ratio_data const &l, real_ref r);
  bool operator>(ratio_data const &l, f64 r);
  bool operator>(f64 l, ratio_data const &r);
  bool operator>(ratio_data const &l, i64 r);
  bool operator>(i64 l, ratio_data const &r);
  bool operator>(bool l, ratio_data const &r);
  bool operator>(ratio_data const &l, bool r);
  bool operator>=(ratio_data const &l, ratio_data const &r);
  bool operator>=(integer_ref l, ratio_data const &r);
  bool operator>=(ratio_data const &l, integer_ref r);
  bool operator>=(real_ref l, ratio_data const &r);
  bool operator>=(ratio_data const &l, real_ref r);
  bool operator>=(ratio_data const &l, f64 r);
  bool operator>=(f64 l, ratio_data const &r);
  bool operator>=(ratio_data const &l, i64 r);
  bool operator>=(i64 l, ratio_data const &r);
  ratio_ref operator+(native_big_integer const &l, ratio_data const &r);
  ratio_ref operator+(ratio_data const &l, native_big_integer const &r);
  ratio_ref operator-(native_big_integer const &l, ratio_data const &r);
  ratio_ref operator-(ratio_data const &l, native_big_integer const &r);
  object_ref operator*(native_big_integer const &l, ratio_data const &r);
  object_ref operator*(ratio_data const &l, native_big_integer const &r);
  object_ref operator/(native_big_integer const &l, ratio_data const &r);
  ratio_ref operator/(ratio_data const &l, native_big_integer const &r);
  bool operator==(native_big_integer const &l, ratio_data const &r);
  bool operator==(ratio_data const &l, native_big_integer const &r);
  bool operator!=(native_big_integer const &l, ratio_data const &r);
  bool operator!=(ratio_data const &l, native_big_integer const &r);
  bool operator<(native_big_integer const &l, ratio_data const &r);
  bool operator<(ratio_data const &l, native_big_integer const &r);
  bool operator<=(native_big_integer const &l, ratio_data const &r);
  bool operator<=(ratio_data const &l, native_big_integer const &r);
  bool operator>(native_big_integer const &l, ratio_data const &r);
  bool operator>(ratio_data const &l, native_big_integer const &r);
  bool operator>=(native_big_integer const &l, ratio_data const &r);
  bool operator>=(ratio_data const &l, native_big_integer const &r);
}
