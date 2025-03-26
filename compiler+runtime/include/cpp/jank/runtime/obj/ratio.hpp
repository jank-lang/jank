#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  struct ratio_data
  {
    ratio_data(native_integer const, native_integer const);
    ratio_data(ratio_data const &) = default;

    native_real to_real() const;
    native_integer to_integer() const;

    native_integer numerator{};
    native_integer denominator{};
  };

  using integer_ptr = native_box<struct integer>;
  using real_ptr = native_box<struct real>;
  using ratio_ptr = native_box<struct ratio>;

  struct ratio : gc
  {
    static constexpr object_type obj_type{ object_type::ratio };
    static constexpr native_bool pointer_free{ true };

    ratio(ratio &&) noexcept = default;
    ratio(ratio const &) = default;
    ratio(ratio_data const &);

    static object_ptr create(native_integer const, native_integer const);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    void to_string(util::string_builder &buff) const;
    jtl::immutable_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::comparable */
    native_integer compare(object const &) const;

    /* behavior::comparable extended */
    native_integer compare(ratio const &) const;

    /* behavior::number_like */
    native_integer to_integer() const;
    native_real to_real() const;

    object base{ obj_type };
    ratio_data data;
  };

  object_ptr operator+(ratio_data const &l, ratio_data const &r);
  ratio_ptr operator+(integer_ptr l, ratio_data const &r);
  ratio_ptr operator+(ratio_data const &l, integer_ptr r);
  native_real operator+(real_ptr l, ratio_data const &r);
  native_real operator+(ratio_data const &l, real_ptr r);
  native_real operator+(ratio_data const &l, native_real r);
  native_real operator+(native_real l, ratio_data const &r);
  ratio_ptr operator+(ratio_data const &l, native_integer r);
  ratio_ptr operator+(native_integer l, ratio_data const &r);
  object_ptr operator-(ratio_data const &l, ratio_data const &r);
  ratio_ptr operator-(integer_ptr l, ratio_data const &r);
  ratio_ptr operator-(ratio_data const &l, integer_ptr r);
  native_real operator-(real_ptr l, ratio_data const &r);
  native_real operator-(ratio_data const &l, real_ptr r);
  native_real operator-(ratio_data const &l, native_real r);
  native_real operator-(native_real l, ratio_data const &r);
  ratio_ptr operator-(ratio_data const &l, native_integer r);
  ratio_ptr operator-(native_integer l, ratio_data const &r);
  object_ptr operator*(ratio_data const &l, ratio_data const &r);
  object_ptr operator*(integer_ptr l, ratio_data const &r);
  object_ptr operator*(ratio_data const &l, integer_ptr r);
  native_real operator*(real_ptr l, ratio_data const &r);
  native_real operator*(ratio_data const &l, real_ptr r);
  native_real operator*(ratio_data const &l, native_real r);
  native_real operator*(native_real l, ratio_data const &r);
  object_ptr operator*(ratio_data const &l, native_integer r);
  object_ptr operator*(native_integer l, ratio_data const &r);
  object_ptr operator/(ratio_data const &l, ratio_data const &r);
  object_ptr operator/(integer_ptr l, ratio_data const &r);
  ratio_ptr operator/(ratio_data const &l, integer_ptr r);
  native_real operator/(real_ptr l, ratio_data const &r);
  native_real operator/(ratio_data const &l, real_ptr r);
  native_real operator/(ratio_data const &l, native_real r);
  native_real operator/(native_real l, ratio_data const &r);
  ratio_ptr operator/(ratio_data const &l, native_integer r);
  object_ptr operator/(native_integer l, ratio_data const &r);
  native_bool operator==(ratio_data const &l, ratio_data const &r);
  native_bool operator==(integer_ptr l, ratio_data const &r);
  native_bool operator==(ratio_data const &l, integer_ptr r);
  native_bool operator==(real_ptr l, ratio_data const &r);
  native_bool operator==(ratio_data const &l, real_ptr r);
  native_bool operator==(ratio_data const &l, native_real r);
  native_bool operator==(native_real l, ratio_data const &r);
  native_bool operator==(ratio_data const &l, native_integer r);
  native_bool operator==(native_integer l, ratio_data const &r);
  native_bool operator<(ratio_data const &l, ratio_data const &r);
  native_bool operator<(integer_ptr l, ratio_data const &r);
  native_bool operator<(ratio_data const &l, integer_ptr r);
  native_bool operator<(real_ptr l, ratio_data const &r);
  native_bool operator<(ratio_data const &l, real_ptr r);
  native_bool operator<(ratio_data const &l, native_real r);
  native_bool operator<(native_real l, ratio_data const &r);
  native_bool operator<(ratio_data const &l, native_integer r);
  native_bool operator<(native_integer l, ratio_data const &r);
  native_bool operator<(native_bool l, ratio_data const &r);
  native_bool operator<(ratio_data const &l, native_bool r);
  native_bool operator<=(ratio_data const &l, ratio_data const &r);
  native_bool operator<=(integer_ptr l, ratio_data const &r);
  native_bool operator<=(ratio_data const &l, integer_ptr r);
  native_bool operator<=(real_ptr l, ratio_data const &r);
  native_bool operator<=(ratio_data const &l, real_ptr r);
  native_bool operator<=(ratio_data const &l, native_real r);
  native_bool operator<=(native_real l, ratio_data const &r);
  native_bool operator<=(ratio_data const &l, native_integer r);
  native_bool operator<=(native_integer l, ratio_data const &r);
  native_bool operator>(ratio_data const &l, ratio_data const &r);
  native_bool operator>(integer_ptr l, ratio_data const &r);
  native_bool operator>(ratio_data const &l, integer_ptr r);
  native_bool operator>(real_ptr l, ratio_data const &r);
  native_bool operator>(ratio_data const &l, real_ptr r);
  native_bool operator>(ratio_data const &l, native_real r);
  native_bool operator>(native_real l, ratio_data const &r);
  native_bool operator>(ratio_data const &l, native_integer r);
  native_bool operator>(native_integer l, ratio_data const &r);
  native_bool operator>(native_bool l, ratio_data const &r);
  native_bool operator>(ratio_data const &l, native_bool r);
  native_bool operator>=(ratio_data const &l, ratio_data const &r);
  native_bool operator>=(integer_ptr l, ratio_data const &r);
  native_bool operator>=(ratio_data const &l, integer_ptr r);
  native_bool operator>=(real_ptr l, ratio_data const &r);
  native_bool operator>=(ratio_data const &l, real_ptr r);
  native_bool operator>=(ratio_data const &l, native_real r);
  native_bool operator>=(native_real l, ratio_data const &r);
  native_bool operator>=(ratio_data const &l, native_integer r);
  native_bool operator>=(native_integer l, ratio_data const &r);
}
