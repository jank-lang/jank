#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/obj/big_integer.hpp>

namespace jank::runtime::obj
{
  static constexpr auto epsilon{ std::numeric_limits<f64>::epsilon() };

  struct ratio_data
  {
    ratio_data(i64 const, i64 const);
    ratio_data(native_big_integer const &, native_big_integer const &);
    ratio_data(big_integer const &, big_integer const &);
    ratio_data(object_ref const, object_ref const);
    ratio_data(ratio_data const &) = default;

    f64 to_real() const;
    i64 to_integer() const;

    native_big_integer numerator{};
    native_big_integer denominator{};
  };

  using integer_ref = oref<struct integer>;
  using real_ref = oref<struct real>;
  using ratio_ref = oref<struct ratio>;

  struct ratio : object
  {
    static constexpr object_type obj_type{ object_type::ratio };
    static constexpr object_behavior obj_behaviors{ object_behavior::compare };
    static constexpr bool pointer_free{ true };

    ratio(ratio &&) noexcept = default;
    ratio(ratio const &) = default;
    ratio(ratio_data const &);

    static object_ref create(i64 const, i64 const);
    static object_ref create(native_big_integer const &, native_big_integer const &);

    /* behavior::object_like */
    bool equal(object const &) const override;
    using object::to_string;
    void to_string(jtl::string_builder &buff) const override;
    uhash to_hash() const override;

    /* behavior::comparable */
    i64 compare(object const &) const override;

    /* behavior::comparable extended */
    i64 compare(ratio const &) const;

    /* behavior::number_like */
    i64 to_integer() const;
    f64 to_real() const;

    /*** XXX: Everything here is immutable after initialization. ***/
    ratio_data data;
  };

  object_ref operator+(ratio_data const &l, ratio_data const &r);
  ratio_data operator+(integer_ref const l, ratio_data const &r);
  ratio_data operator+(ratio_data const &l, integer_ref const r);
  f64 operator+(real_ref const l, ratio_data const &r);
  f64 operator+(ratio_data const &l, real_ref const r);

  template <typename T>
  requires std::is_floating_point_v<T>
  f64 operator+(T const l, ratio_data const &r)
  {
    return l + r.to_real();
  }

  template <typename T>
  requires std::is_floating_point_v<T>
  f64 operator+(ratio_data const &l, T const r)
  {
    return l.to_real() + r;
  }

  template <typename T>
  requires std::is_integral_v<T>
  ratio_data operator+(T const l, ratio_data const &r)
  {
    return ratio_data(r.numerator + (l * r.denominator), r.denominator);
  }

  template <typename T>
  requires std::is_integral_v<T>
  ratio_data operator+(ratio_data const &l, T const r)
  {
    return ratio_data(l.numerator + (r * l.denominator), l.denominator);
  }

  object_ref operator-(ratio_data const &l, ratio_data const &r);
  ratio_data operator-(integer_ref const l, ratio_data const &r);
  ratio_data operator-(ratio_data const &l, integer_ref const r);
  f64 operator-(real_ref const l, ratio_data const &r);
  f64 operator-(ratio_data const &l, real_ref const r);

  template <typename T>
  requires std::is_floating_point_v<T>
  f64 operator-(T const l, ratio_data const &r)
  {
    return l - r.to_real();
  }

  template <typename T>
  requires std::is_floating_point_v<T>
  f64 operator-(ratio_data const &l, T const r)
  {
    return l.to_real() - r;
  }

  template <typename T>
  requires std::is_integral_v<T>
  ratio_data operator-(T const l, ratio_data const &r)
  {
    return ratio_data((l * r.denominator) - r.numerator, r.denominator);
  }

  template <typename T>
  requires std::is_integral_v<T>
  ratio_data operator-(ratio_data const &l, T const r)
  {
    return ratio_data(l.numerator - (r * l.denominator), l.denominator);
  }

  object_ref operator*(ratio_data const &l, ratio_data const &r);
  object_ref operator*(integer_ref const l, ratio_data const &r);
  object_ref operator*(ratio_data const &l, integer_ref const r);
  f64 operator*(real_ref const l, ratio_data const &r);
  f64 operator*(ratio_data const &l, real_ref const r);

  template <typename T>
  requires std::is_floating_point_v<T>
  f64 operator*(T const l, ratio_data const &r)
  {
    return l * r.to_real();
  }

  template <typename T>
  requires std::is_floating_point_v<T>
  f64 operator*(ratio_data const &l, T const r)
  {
    return l.to_real() * r;
  }

  template <typename T>
  requires std::is_integral_v<T>
  object_ref operator*(T const l, ratio_data const &r)
  {
    return r * ratio_data(l, 1ll);
  }

  template <typename T>
  requires std::is_integral_v<T>
  object_ref operator*(ratio_data const &l, T const r)
  {
    return l * ratio_data(r, 1ll);
  }

  object_ref operator/(ratio_data const &l, ratio_data const &r);
  object_ref operator/(integer_ref const l, ratio_data const &r);
  ratio_ref operator/(ratio_data const &l, integer_ref const r);
  f64 operator/(real_ref const l, ratio_data const &r);
  f64 operator/(ratio_data const &l, real_ref const r);

  template <typename T>
  requires std::is_floating_point_v<T>
  f64 operator/(T const l, ratio_data const &r)
  {
    return l / r.to_real();
  }

  template <typename T>
  requires std::is_floating_point_v<T>
  f64 operator/(ratio_data const &l, T const r)
  {
    return l.to_real() / r;
  }

  template <typename T>
  requires std::is_integral_v<T>
  object_ref operator/(T const l, ratio_data const &r)
  {
    return ratio_data(l, 1ll) / r;
  }

  template <typename T>
  requires std::is_integral_v<T>
  ratio_ref operator/(ratio_data const &l, T const r)
  {
    return make_box<ratio>(ratio_data(l.numerator, l.denominator * r));
  }

  bool operator==(ratio_data const &l, ratio_data const &r);
  bool operator==(integer_ref const l, ratio_data const &r);
  bool operator==(ratio_data const &l, integer_ref const r);
  bool operator==(real_ref const l, ratio_data const &r);
  bool operator==(ratio_data const &l, real_ref const r);

  template <typename T>
  requires std::is_floating_point_v<T>
  bool operator==(T const l, ratio_data const &r)
  {
    return std::fabs(r - l) < epsilon;
  }

  template <typename T>
  requires std::is_floating_point_v<T>
  bool operator==(ratio_data const &l, T const r)
  {
    return std::fabs(l - r) < epsilon;
  }

  template <typename T>
  requires std::is_integral_v<T>
  bool operator==(T const l, ratio_data const &r)
  {
    return l * r.denominator == r.numerator;
  }

  template <typename T>
  requires std::is_integral_v<T>
  bool operator==(ratio_data const &l, T const r)
  {
    return l.numerator == r * l.denominator;
  }

  bool operator<(ratio_data const &l, ratio_data const &r);
  bool operator<(integer_ref const l, ratio_data const &r);
  bool operator<(ratio_data const &l, integer_ref const r);
  bool operator<(real_ref const l, ratio_data const &r);
  bool operator<(ratio_data const &l, real_ref const r);

  template <typename T>
  requires std::is_floating_point_v<T>
  bool operator<(T const l, ratio_data const &r)
  {
    return l < r.to_real();
  }

  template <typename T>
  requires std::is_floating_point_v<T>
  bool operator<(ratio_data const &l, T const r)
  {
    return l.to_real() < r;
  }

  template <typename T>
  requires std::is_integral_v<T>
  bool operator<(T const l, ratio_data const &r)
  {
    return l * r.denominator < r.numerator;
  }

  template <typename T>
  requires std::is_integral_v<T>
  bool operator<(ratio_data const &l, T const r)
  {
    return l.numerator < r * l.denominator;
  }

  bool operator<=(ratio_data const &l, ratio_data const &r);
  bool operator<=(integer_ref const l, ratio_data const &r);
  bool operator<=(ratio_data const &l, integer_ref const r);
  bool operator<=(real_ref const l, ratio_data const &r);
  bool operator<=(ratio_data const &l, real_ref const r);

  template <typename T>
  requires std::is_floating_point_v<T>
  bool operator<=(T const l, ratio_data const &r)
  {
    return l <= r.to_real();
  }

  template <typename T>
  requires std::is_floating_point_v<T>
  bool operator<=(ratio_data const &l, T const r)
  {
    return l.to_real() <= r;
  }

  template <typename T>
  requires std::is_integral_v<T>
  bool operator<=(T const l, ratio_data const &r)
  {
    return l * r.denominator <= r.numerator;
  }

  template <typename T>
  requires std::is_integral_v<T>
  bool operator<=(ratio_data const &l, T const r)
  {
    return l.numerator <= r * l.denominator;
  }

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

  native_big_decimal operator+(native_big_decimal const &l, ratio_data const &r);
  native_big_decimal operator+(ratio_data const &l, native_big_decimal const &r);
  native_big_decimal operator-(native_big_decimal const &l, ratio_data const &r);
  native_big_decimal operator-(ratio_data const &l, native_big_decimal const &r);
  native_big_decimal operator*(native_big_decimal const &l, ratio_data const &r);
  native_big_decimal operator*(ratio_data const &l, native_big_decimal const &r);
  native_big_decimal operator/(native_big_decimal const &l, ratio_data const &r);
  native_big_decimal operator/(ratio_data const &l, native_big_decimal const &r);

  bool operator==(native_big_decimal const &l, ratio_data const &r);
  bool operator==(ratio_data const &l, native_big_decimal const &r);
  bool operator!=(native_big_decimal const &l, ratio_data const &r);
  bool operator!=(ratio_data const &l, native_big_decimal const &r);
  bool operator<(native_big_decimal const &l, ratio_data const &r);
  bool operator<(ratio_data const &l, native_big_decimal const &r);
  bool operator<=(native_big_decimal const &l, ratio_data const &r);
  bool operator<=(ratio_data const &l, native_big_decimal const &r);
}
