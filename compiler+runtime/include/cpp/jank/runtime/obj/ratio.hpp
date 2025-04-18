#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  struct ratio_data
  {
    ratio_data(i64 const, i64 const);
    ratio_data(ratio_data const &) = default;

    native_real to_real() const;
    i64 to_integer() const;

    i64 numerator{};
    i64 denominator{};
  };

  using integer_ref = oref<struct integer>;
  using real_ref = oref<struct real>;
  using ratio_ref = oref<struct ratio>;

  struct ratio : gc
  {
    static constexpr object_type obj_type{ object_type::ratio };
    static constexpr native_bool pointer_free{ true };

    ratio(ratio &&) noexcept = default;
    ratio(ratio const &) = default;
    ratio(ratio_data const &);

    static object_ref create(i64 const, i64 const);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    void to_string(util::string_builder &buff) const;
    jtl::immutable_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::comparable */
    i64 compare(object const &) const;

    /* behavior::comparable extended */
    i64 compare(ratio const &) const;

    /* behavior::number_like */
    i64 to_integer() const;
    native_real to_real() const;

    object base{ obj_type };
    ratio_data data;
  };

  object_ref operator+(ratio_data const &l, ratio_data const &r);
  ratio_ref operator+(integer_ref l, ratio_data const &r);
  ratio_ref operator+(ratio_data const &l, integer_ref r);
  native_real operator+(real_ref l, ratio_data const &r);
  native_real operator+(ratio_data const &l, real_ref r);
  native_real operator+(ratio_data const &l, native_real r);
  native_real operator+(native_real l, ratio_data const &r);
  ratio_ref operator+(ratio_data const &l, i64 r);
  ratio_ref operator+(i64 l, ratio_data const &r);
  object_ref operator-(ratio_data const &l, ratio_data const &r);
  ratio_ref operator-(integer_ref l, ratio_data const &r);
  ratio_ref operator-(ratio_data const &l, integer_ref r);
  native_real operator-(real_ref l, ratio_data const &r);
  native_real operator-(ratio_data const &l, real_ref r);
  native_real operator-(ratio_data const &l, native_real r);
  native_real operator-(native_real l, ratio_data const &r);
  ratio_ref operator-(ratio_data const &l, i64 r);
  ratio_ref operator-(i64 l, ratio_data const &r);
  object_ref operator*(ratio_data const &l, ratio_data const &r);
  object_ref operator*(integer_ref l, ratio_data const &r);
  object_ref operator*(ratio_data const &l, integer_ref r);
  native_real operator*(real_ref l, ratio_data const &r);
  native_real operator*(ratio_data const &l, real_ref r);
  native_real operator*(ratio_data const &l, native_real r);
  native_real operator*(native_real l, ratio_data const &r);
  object_ref operator*(ratio_data const &l, i64 r);
  object_ref operator*(i64 l, ratio_data const &r);
  object_ref operator/(ratio_data const &l, ratio_data const &r);
  object_ref operator/(integer_ref l, ratio_data const &r);
  ratio_ref operator/(ratio_data const &l, integer_ref r);
  native_real operator/(real_ref l, ratio_data const &r);
  native_real operator/(ratio_data const &l, real_ref r);
  native_real operator/(ratio_data const &l, native_real r);
  native_real operator/(native_real l, ratio_data const &r);
  ratio_ref operator/(ratio_data const &l, i64 r);
  object_ref operator/(i64 l, ratio_data const &r);
  native_bool operator==(ratio_data const &l, ratio_data const &r);
  native_bool operator==(integer_ref l, ratio_data const &r);
  native_bool operator==(ratio_data const &l, integer_ref r);
  native_bool operator==(real_ref l, ratio_data const &r);
  native_bool operator==(ratio_data const &l, real_ref r);
  native_bool operator==(ratio_data const &l, native_real r);
  native_bool operator==(native_real l, ratio_data const &r);
  native_bool operator==(ratio_data const &l, i64 r);
  native_bool operator==(i64 l, ratio_data const &r);
  native_bool operator<(ratio_data const &l, ratio_data const &r);
  native_bool operator<(integer_ref l, ratio_data const &r);
  native_bool operator<(ratio_data const &l, integer_ref r);
  native_bool operator<(real_ref l, ratio_data const &r);
  native_bool operator<(ratio_data const &l, real_ref r);
  native_bool operator<(ratio_data const &l, native_real r);
  native_bool operator<(native_real l, ratio_data const &r);
  native_bool operator<(ratio_data const &l, i64 r);
  native_bool operator<(i64 l, ratio_data const &r);
  native_bool operator<(native_bool l, ratio_data const &r);
  native_bool operator<(ratio_data const &l, native_bool r);
  native_bool operator<=(ratio_data const &l, ratio_data const &r);
  native_bool operator<=(integer_ref l, ratio_data const &r);
  native_bool operator<=(ratio_data const &l, integer_ref r);
  native_bool operator<=(real_ref l, ratio_data const &r);
  native_bool operator<=(ratio_data const &l, real_ref r);
  native_bool operator<=(ratio_data const &l, native_real r);
  native_bool operator<=(native_real l, ratio_data const &r);
  native_bool operator<=(ratio_data const &l, i64 r);
  native_bool operator<=(i64 l, ratio_data const &r);
  native_bool operator>(ratio_data const &l, ratio_data const &r);
  native_bool operator>(integer_ref l, ratio_data const &r);
  native_bool operator>(ratio_data const &l, integer_ref r);
  native_bool operator>(real_ref l, ratio_data const &r);
  native_bool operator>(ratio_data const &l, real_ref r);
  native_bool operator>(ratio_data const &l, native_real r);
  native_bool operator>(native_real l, ratio_data const &r);
  native_bool operator>(ratio_data const &l, i64 r);
  native_bool operator>(i64 l, ratio_data const &r);
  native_bool operator>(native_bool l, ratio_data const &r);
  native_bool operator>(ratio_data const &l, native_bool r);
  native_bool operator>=(ratio_data const &l, ratio_data const &r);
  native_bool operator>=(integer_ref l, ratio_data const &r);
  native_bool operator>=(ratio_data const &l, integer_ref r);
  native_bool operator>=(real_ref l, ratio_data const &r);
  native_bool operator>=(ratio_data const &l, real_ref r);
  native_bool operator>=(ratio_data const &l, native_real r);
  native_bool operator>=(native_real l, ratio_data const &r);
  native_bool operator>=(ratio_data const &l, i64 r);
  native_bool operator>=(i64 l, ratio_data const &r);
}
