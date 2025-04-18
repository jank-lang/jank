#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime
{
  namespace obj
  {
    using integer_ref = oref<struct integer>;
    using real_ref = oref<struct real>;
    using ratio_ref = oref<struct ratio>;
  }

  object_ref add(object_ref l, object_ref r);
  object_ref add(obj::integer_ref l, object_ref r);
  object_ref add(object_ref l, obj::integer_ref r);
  object_ref add(object_ref l, obj::ratio_ref r);
  i64 add(obj::integer_ref l, obj::integer_ref r);
  native_real add(obj::real_ref l, obj::real_ref r);
  native_real add(obj::real_ref l, object_ref r);
  native_real add(object_ref l, obj::real_ref r);
  native_real add(obj::real_ref l, obj::integer_ref r);
  native_real add(obj::integer_ref l, obj::real_ref r);

  native_real add(object_ref l, native_real r);
  native_real add(native_real l, object_ref r);
  native_real add(native_real l, native_real r);

  native_real add(i64 l, native_real r);
  native_real add(native_real l, i64 r);

  object_ref add(object_ref l, i64 r);
  object_ref add(i64 l, object_ref r);
  i64 add(i64 l, i64 r);

  obj::ratio_ref add(obj::ratio_ref l, obj::ratio_ref r);
  object_ref add(obj::ratio_ref l, obj::integer_ref r);
  obj::ratio_ref add(obj::integer_ref l, obj::ratio_ref r);

  object_ref sub(object_ref l, object_ref r);
  object_ref sub(obj::integer_ref l, object_ref r);
  object_ref sub(object_ref l, obj::integer_ref r);
  i64 sub(obj::integer_ref l, obj::integer_ref r);
  native_real sub(obj::real_ref l, obj::real_ref r);
  native_real sub(obj::real_ref l, object_ref r);
  native_real sub(object_ref l, obj::real_ref r);
  native_real sub(obj::real_ref l, obj::integer_ref r);
  native_real sub(obj::integer_ref l, obj::real_ref r);

  native_real sub(object_ref l, native_real r);
  native_real sub(native_real l, object_ref r);
  native_real sub(native_real l, native_real r);

  native_real sub(i64 l, native_real r);
  native_real sub(native_real l, i64 r);

  object_ref sub(object_ref l, i64 r);
  object_ref sub(i64 l, object_ref r);
  i64 sub(i64 l, i64 r);

  object_ref div(object_ref l, object_ref r);
  object_ref div(obj::integer_ref l, object_ref r);
  object_ref div(object_ref l, obj::integer_ref r);
  i64 div(obj::integer_ref l, obj::integer_ref r);
  native_real div(obj::real_ref l, obj::real_ref r);
  native_real div(obj::real_ref l, object_ref r);
  native_real div(object_ref l, obj::real_ref r);
  native_real div(obj::real_ref l, obj::integer_ref r);
  native_real div(obj::integer_ref l, obj::real_ref r);

  native_real div(object_ref l, native_real r);
  native_real div(native_real l, object_ref r);
  native_real div(native_real l, native_real r);

  native_real div(i64 l, native_real r);
  native_real div(native_real l, i64 r);

  object_ref div(object_ref l, i64 r);
  object_ref div(i64 l, object_ref r);
  i64 div(i64 l, i64 r);

  object_ref mul(object_ref l, object_ref r);
  object_ref mul(obj::integer_ref l, object_ref r);
  object_ref mul(object_ref l, obj::integer_ref r);
  i64 mul(obj::integer_ref l, obj::integer_ref r);
  native_real mul(obj::real_ref l, obj::real_ref r);
  native_real mul(obj::real_ref l, object_ref r);
  native_real mul(object_ref l, obj::real_ref r);
  native_real mul(obj::real_ref l, obj::integer_ref r);
  native_real mul(obj::integer_ref l, obj::real_ref r);

  native_real mul(object_ref l, native_real r);
  native_real mul(native_real l, object_ref r);
  native_real mul(native_real l, native_real r);

  native_real mul(i64 l, native_real r);
  native_real mul(native_real l, i64 r);

  object_ref mul(object_ref l, i64 r);
  object_ref mul(i64 l, object_ref r);
  i64 mul(i64 l, i64 r);

  native_bool lt(object_ref l, object_ref r);
  native_bool lt(obj::integer_ref l, object_ref r);
  native_bool lt(object_ref l, obj::integer_ref r);
  native_bool lt(obj::integer_ref const l, obj::integer_ref const r);
  native_bool lt(obj::real_ref const l, obj::real_ref const r);
  native_bool lt(obj::real_ref l, object_ref r);
  native_bool lt(object_ref l, obj::real_ref r);
  native_bool lt(obj::real_ref l, obj::integer_ref r);
  native_bool lt(obj::integer_ref l, obj::real_ref r);

  native_bool lt(object_ref l, native_real r);
  native_bool lt(native_real l, object_ref r);
  native_bool lt(native_real l, native_real r);

  native_bool lt(i64 l, native_real r);
  native_bool lt(native_real l, i64 r);

  native_bool lt(object_ref l, i64 r);
  native_bool lt(i64 l, object_ref r);
  native_bool lt(i64 l, i64 r);

  native_bool lte(object_ref l, object_ref r);
  native_bool lte(obj::integer_ref l, object_ref r);
  native_bool lte(object_ref l, obj::integer_ref r);
  native_bool lte(obj::integer_ref const l, obj::integer_ref const r);
  native_bool lte(obj::real_ref const l, obj::real_ref const r);
  native_bool lte(obj::real_ref l, object_ref r);
  native_bool lte(object_ref l, obj::real_ref r);
  native_bool lte(obj::real_ref l, obj::integer_ref r);
  native_bool lte(obj::integer_ref l, obj::real_ref r);

  native_bool lte(object_ref l, native_real r);
  native_bool lte(native_real l, object_ref r);
  native_bool lte(native_real l, native_real r);

  native_bool lte(i64 l, native_real r);
  native_bool lte(native_real l, i64 r);

  native_bool lte(object_ref l, i64 r);
  native_bool lte(i64 l, object_ref r);
  native_bool lte(i64 l, i64 r);

  object_ref min(object_ref l, object_ref r);
  object_ref min(obj::integer_ref l, object_ref r);
  object_ref min(object_ref l, obj::integer_ref r);
  i64 min(obj::integer_ref l, obj::integer_ref r);
  native_real min(obj::real_ref l, obj::real_ref r);
  native_real min(obj::real_ref l, object_ref r);
  native_real min(object_ref l, obj::real_ref r);
  native_real min(obj::real_ref l, obj::integer_ref r);
  native_real min(obj::integer_ref l, obj::real_ref r);

  native_real min(object_ref l, native_real r);
  native_real min(native_real l, object_ref r);
  native_real min(native_real l, native_real r);

  native_real min(i64 l, native_real r);
  native_real min(native_real l, i64 r);

  object_ref min(object_ref l, i64 r);
  object_ref min(i64 l, object_ref r);
  i64 min(i64 l, i64 r);

  object_ref max(object_ref l, object_ref r);
  object_ref max(obj::integer_ref l, object_ref r);
  object_ref max(object_ref l, obj::integer_ref r);
  i64 max(obj::integer_ref l, obj::integer_ref r);
  native_real max(obj::real_ref l, obj::real_ref r);
  native_real max(obj::real_ref l, object_ref r);
  native_real max(object_ref l, obj::real_ref r);
  native_real max(obj::real_ref l, obj::integer_ref r);
  native_real max(obj::integer_ref l, obj::real_ref r);

  native_real max(object_ref l, native_real r);
  native_real max(native_real l, object_ref r);
  native_real max(native_real l, native_real r);

  native_real max(i64 l, native_real r);
  native_real max(native_real l, i64 r);

  object_ref max(object_ref l, i64 r);
  object_ref max(i64 l, object_ref r);
  i64 max(i64 l, i64 r);

  object_ref abs(object_ref l);
  i64 abs(obj::integer_ref l);
  native_real abs(obj::real_ref l);
  i64 abs(i64 l);
  native_real abs(native_real l);

  native_real tan(object_ref l);

  native_real sqrt(object_ref l);
  native_real sqrt(obj::integer_ref l);
  native_real sqrt(obj::real_ref l);
  native_real sqrt(i64 l);
  native_real sqrt(native_real l);

  native_real pow(object_ref l, object_ref r);
  native_real pow(obj::integer_ref l, object_ref r);
  native_real pow(object_ref l, obj::integer_ref r);
  native_real pow(obj::integer_ref l, obj::integer_ref r);
  native_real pow(obj::real_ref l, obj::real_ref r);
  native_real pow(obj::real_ref l, object_ref r);
  native_real pow(object_ref l, obj::real_ref r);
  native_real pow(obj::real_ref l, obj::integer_ref r);
  native_real pow(obj::integer_ref l, obj::real_ref r);

  native_real pow(object_ref l, native_real r);
  native_real pow(native_real l, object_ref r);
  native_real pow(native_real l, native_real r);

  native_real pow(i64 l, native_real r);
  native_real pow(native_real l, i64 r);

  native_real pow(object_ref l, i64 r);
  native_real pow(i64 l, object_ref r);
  native_real pow(i64 l, i64 r);

  object_ref rem(object_ref l, object_ref r);
  object_ref quot(object_ref l, object_ref r);
  object_ref inc(object_ref l);
  object_ref dec(object_ref l);

  native_bool is_zero(object_ref l);
  native_bool is_pos(object_ref l);
  native_bool is_neg(object_ref l);
  native_bool is_even(object_ref l);
  native_bool is_odd(object_ref l);

  native_bool is_equiv(object_ref l, object_ref r);

  i64 bit_not(object_ref l);
  i64 bit_and(object_ref l, object_ref r);
  i64 bit_or(object_ref l, object_ref r);
  i64 bit_xor(object_ref l, object_ref r);
  i64 bit_and_not(object_ref l, object_ref r);
  i64 bit_clear(object_ref l, object_ref r);
  i64 bit_set(object_ref l, object_ref r);
  i64 bit_flip(object_ref l, object_ref r);
  native_bool bit_test(object_ref l, object_ref r);
  i64 bit_shift_left(object_ref l, object_ref r);
  i64 bit_shift_right(object_ref l, object_ref r);
  i64 bit_unsigned_shift_right(object_ref l, object_ref r);

  native_real rand();

  i64 numerator(object_ref o);
  i64 denominator(object_ref o);

  i64 to_int(object_ref l);
  i64 to_int(obj::integer_ref l);
  i64 to_int(obj::real_ref l);
  i64 to_int(i64 l);
  i64 to_int(native_real l);

  native_real to_real(object_ref o);

  native_bool is_number(object_ref o);
  native_bool is_integer(object_ref o);
  native_bool is_real(object_ref o);
  native_bool is_ratio(object_ref o);
  native_bool is_boolean(object_ref o);
  native_bool is_nan(object_ref o);
  native_bool is_infinite(object_ref o);

  i64 parse_long(object_ref o);
  native_real parse_double(object_ref o);
}
