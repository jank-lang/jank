#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime
{
  namespace obj
  {
    using integer_ref = jtl::oref<struct integer>;
    using real_ref = jtl::oref<struct real>;
    using ratio_ref = jtl::oref<struct ratio>;
  }

  object_ptr add(object_ptr l, object_ptr r);
  object_ptr add(obj::integer_ref l, object_ptr r);
  object_ptr add(object_ptr l, obj::integer_ref r);
  object_ptr add(object_ptr l, obj::ratio_ref r);
  native_integer add(obj::integer_ref l, obj::integer_ref r);
  native_real add(obj::real_ref l, obj::real_ref r);
  native_real add(obj::real_ref l, object_ptr r);
  native_real add(object_ptr l, obj::real_ref r);
  native_real add(obj::real_ref l, obj::integer_ref r);
  native_real add(obj::integer_ref l, obj::real_ref r);

  native_real add(object_ptr l, native_real r);
  native_real add(native_real l, object_ptr r);
  native_real add(native_real l, native_real r);

  native_real add(native_integer l, native_real r);
  native_real add(native_real l, native_integer r);

  object_ptr add(object_ptr l, native_integer r);
  object_ptr add(native_integer l, object_ptr r);
  native_integer add(native_integer l, native_integer r);

  obj::ratio_ref add(obj::ratio_ref l, obj::ratio_ref r);
  object_ptr add(obj::ratio_ref l, obj::integer_ref r);
  obj::ratio_ref add(obj::integer_ref l, obj::ratio_ref r);

  object_ptr sub(object_ptr l, object_ptr r);
  object_ptr sub(obj::integer_ref l, object_ptr r);
  object_ptr sub(object_ptr l, obj::integer_ref r);
  native_integer sub(obj::integer_ref l, obj::integer_ref r);
  native_real sub(obj::real_ref l, obj::real_ref r);
  native_real sub(obj::real_ref l, object_ptr r);
  native_real sub(object_ptr l, obj::real_ref r);
  native_real sub(obj::real_ref l, obj::integer_ref r);
  native_real sub(obj::integer_ref l, obj::real_ref r);

  native_real sub(object_ptr l, native_real r);
  native_real sub(native_real l, object_ptr r);
  native_real sub(native_real l, native_real r);

  native_real sub(native_integer l, native_real r);
  native_real sub(native_real l, native_integer r);

  object_ptr sub(object_ptr l, native_integer r);
  object_ptr sub(native_integer l, object_ptr r);
  native_integer sub(native_integer l, native_integer r);

  object_ptr div(object_ptr l, object_ptr r);
  object_ptr div(obj::integer_ref l, object_ptr r);
  object_ptr div(object_ptr l, obj::integer_ref r);
  native_integer div(obj::integer_ref l, obj::integer_ref r);
  native_real div(obj::real_ref l, obj::real_ref r);
  native_real div(obj::real_ref l, object_ptr r);
  native_real div(object_ptr l, obj::real_ref r);
  native_real div(obj::real_ref l, obj::integer_ref r);
  native_real div(obj::integer_ref l, obj::real_ref r);

  native_real div(object_ptr l, native_real r);
  native_real div(native_real l, object_ptr r);
  native_real div(native_real l, native_real r);

  native_real div(native_integer l, native_real r);
  native_real div(native_real l, native_integer r);

  object_ptr div(object_ptr l, native_integer r);
  object_ptr div(native_integer l, object_ptr r);
  native_integer div(native_integer l, native_integer r);

  object_ptr mul(object_ptr l, object_ptr r);
  object_ptr mul(obj::integer_ref l, object_ptr r);
  object_ptr mul(object_ptr l, obj::integer_ref r);
  native_integer mul(obj::integer_ref l, obj::integer_ref r);
  native_real mul(obj::real_ref l, obj::real_ref r);
  native_real mul(obj::real_ref l, object_ptr r);
  native_real mul(object_ptr l, obj::real_ref r);
  native_real mul(obj::real_ref l, obj::integer_ref r);
  native_real mul(obj::integer_ref l, obj::real_ref r);

  native_real mul(object_ptr l, native_real r);
  native_real mul(native_real l, object_ptr r);
  native_real mul(native_real l, native_real r);

  native_real mul(native_integer l, native_real r);
  native_real mul(native_real l, native_integer r);

  object_ptr mul(object_ptr l, native_integer r);
  object_ptr mul(native_integer l, object_ptr r);
  native_integer mul(native_integer l, native_integer r);

  native_bool lt(object_ptr l, object_ptr r);
  native_bool lt(obj::integer_ref l, object_ptr r);
  native_bool lt(object_ptr l, obj::integer_ref r);
  native_bool lt(obj::integer_ref const l, obj::integer_ref const r);
  native_bool lt(obj::real_ref const l, obj::real_ref const r);
  native_bool lt(obj::real_ref l, object_ptr r);
  native_bool lt(object_ptr l, obj::real_ref r);
  native_bool lt(obj::real_ref l, obj::integer_ref r);
  native_bool lt(obj::integer_ref l, obj::real_ref r);

  native_bool lt(object_ptr l, native_real r);
  native_bool lt(native_real l, object_ptr r);
  native_bool lt(native_real l, native_real r);

  native_bool lt(native_integer l, native_real r);
  native_bool lt(native_real l, native_integer r);

  native_bool lt(object_ptr l, native_integer r);
  native_bool lt(native_integer l, object_ptr r);
  native_bool lt(native_integer l, native_integer r);

  native_bool lte(object_ptr l, object_ptr r);
  native_bool lte(obj::integer_ref l, object_ptr r);
  native_bool lte(object_ptr l, obj::integer_ref r);
  native_bool lte(obj::integer_ref const l, obj::integer_ref const r);
  native_bool lte(obj::real_ref const l, obj::real_ref const r);
  native_bool lte(obj::real_ref l, object_ptr r);
  native_bool lte(object_ptr l, obj::real_ref r);
  native_bool lte(obj::real_ref l, obj::integer_ref r);
  native_bool lte(obj::integer_ref l, obj::real_ref r);

  native_bool lte(object_ptr l, native_real r);
  native_bool lte(native_real l, object_ptr r);
  native_bool lte(native_real l, native_real r);

  native_bool lte(native_integer l, native_real r);
  native_bool lte(native_real l, native_integer r);

  native_bool lte(object_ptr l, native_integer r);
  native_bool lte(native_integer l, object_ptr r);
  native_bool lte(native_integer l, native_integer r);

  object_ptr min(object_ptr l, object_ptr r);
  object_ptr min(obj::integer_ref l, object_ptr r);
  object_ptr min(object_ptr l, obj::integer_ref r);
  native_integer min(obj::integer_ref l, obj::integer_ref r);
  native_real min(obj::real_ref l, obj::real_ref r);
  native_real min(obj::real_ref l, object_ptr r);
  native_real min(object_ptr l, obj::real_ref r);
  native_real min(obj::real_ref l, obj::integer_ref r);
  native_real min(obj::integer_ref l, obj::real_ref r);

  native_real min(object_ptr l, native_real r);
  native_real min(native_real l, object_ptr r);
  native_real min(native_real l, native_real r);

  native_real min(native_integer l, native_real r);
  native_real min(native_real l, native_integer r);

  object_ptr min(object_ptr l, native_integer r);
  object_ptr min(native_integer l, object_ptr r);
  native_integer min(native_integer l, native_integer r);

  object_ptr max(object_ptr l, object_ptr r);
  object_ptr max(obj::integer_ref l, object_ptr r);
  object_ptr max(object_ptr l, obj::integer_ref r);
  native_integer max(obj::integer_ref l, obj::integer_ref r);
  native_real max(obj::real_ref l, obj::real_ref r);
  native_real max(obj::real_ref l, object_ptr r);
  native_real max(object_ptr l, obj::real_ref r);
  native_real max(obj::real_ref l, obj::integer_ref r);
  native_real max(obj::integer_ref l, obj::real_ref r);

  native_real max(object_ptr l, native_real r);
  native_real max(native_real l, object_ptr r);
  native_real max(native_real l, native_real r);

  native_real max(native_integer l, native_real r);
  native_real max(native_real l, native_integer r);

  object_ptr max(object_ptr l, native_integer r);
  object_ptr max(native_integer l, object_ptr r);
  native_integer max(native_integer l, native_integer r);

  object_ptr abs(object_ptr l);
  native_integer abs(obj::integer_ref l);
  native_real abs(obj::real_ref l);
  native_integer abs(native_integer l);
  native_real abs(native_real l);

  native_real tan(object_ptr l);

  native_real sqrt(object_ptr l);
  native_real sqrt(obj::integer_ref l);
  native_real sqrt(obj::real_ref l);
  native_real sqrt(native_integer l);
  native_real sqrt(native_real l);

  native_real pow(object_ptr l, object_ptr r);
  native_real pow(obj::integer_ref l, object_ptr r);
  native_real pow(object_ptr l, obj::integer_ref r);
  native_real pow(obj::integer_ref l, obj::integer_ref r);
  native_real pow(obj::real_ref l, obj::real_ref r);
  native_real pow(obj::real_ref l, object_ptr r);
  native_real pow(object_ptr l, obj::real_ref r);
  native_real pow(obj::real_ref l, obj::integer_ref r);
  native_real pow(obj::integer_ref l, obj::real_ref r);

  native_real pow(object_ptr l, native_real r);
  native_real pow(native_real l, object_ptr r);
  native_real pow(native_real l, native_real r);

  native_real pow(native_integer l, native_real r);
  native_real pow(native_real l, native_integer r);

  native_real pow(object_ptr l, native_integer r);
  native_real pow(native_integer l, object_ptr r);
  native_real pow(native_integer l, native_integer r);

  object_ptr rem(object_ptr l, object_ptr r);
  object_ptr quot(object_ptr l, object_ptr r);
  object_ptr inc(object_ptr l);
  object_ptr dec(object_ptr l);

  native_bool is_zero(object_ptr l);
  native_bool is_pos(object_ptr l);
  native_bool is_neg(object_ptr l);
  native_bool is_even(object_ptr l);
  native_bool is_odd(object_ptr l);

  native_bool is_equiv(object_ptr l, object_ptr r);

  native_integer bit_not(object_ptr l);
  native_integer bit_and(object_ptr l, object_ptr r);
  native_integer bit_or(object_ptr l, object_ptr r);
  native_integer bit_xor(object_ptr l, object_ptr r);
  native_integer bit_and_not(object_ptr l, object_ptr r);
  native_integer bit_clear(object_ptr l, object_ptr r);
  native_integer bit_set(object_ptr l, object_ptr r);
  native_integer bit_flip(object_ptr l, object_ptr r);
  native_bool bit_test(object_ptr l, object_ptr r);
  native_integer bit_shift_left(object_ptr l, object_ptr r);
  native_integer bit_shift_right(object_ptr l, object_ptr r);
  native_integer bit_unsigned_shift_right(object_ptr l, object_ptr r);

  native_real rand();

  native_integer numerator(object_ptr o);
  native_integer denominator(object_ptr o);

  native_integer to_int(object_ptr l);
  native_integer to_int(obj::integer_ref l);
  native_integer to_int(obj::real_ref l);
  native_integer to_int(native_integer l);
  native_integer to_int(native_real l);

  native_real to_real(object_ptr o);

  native_bool is_number(object_ptr o);
  native_bool is_integer(object_ptr o);
  native_bool is_real(object_ptr o);
  native_bool is_ratio(object_ptr o);
  native_bool is_boolean(object_ptr o);
  native_bool is_nan(object_ptr o);
  native_bool is_infinite(object_ptr o);

  native_integer parse_long(object_ptr o);
  native_real parse_double(object_ptr o);
}
