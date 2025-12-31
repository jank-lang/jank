#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime
{
  namespace obj
  {
    using integer_ref = oref<struct integer>;
    using real_ref = oref<struct real>;
    using ratio_ref = oref<struct ratio>;
    using big_integer_ref = oref<struct big_integer>;
    using big_decimal_ref = oref<struct big_decimal>;
  }

  object_ref add(object_ref const l, object_ref const r);
  object_ref add(obj::integer_ref const l, object_ref const r);
  object_ref add(object_ref const l, obj::integer_ref const r);
  object_ref add(object_ref const l, obj::ratio_ref const r);
  object_ref add(obj::big_decimal_ref const l, object_ref const r);
  object_ref add(object_ref const l, obj::big_decimal_ref const r);
  i64 add(obj::integer_ref const l, obj::integer_ref const r);
  f64 add(obj::real_ref const l, obj::real_ref const r);
  object_ref add(obj::real_ref const l, object_ref const r);
  object_ref add(object_ref const l, obj::real_ref const r);
  f64 add(obj::real_ref const l, obj::integer_ref const r);
  f64 add(obj::integer_ref const l, obj::real_ref const r);

  object_ref add(object_ref const l, f64 r);
  object_ref add(f64 l, object_ref const r);
  f64 add(f64 l, f64 r);

  f64 add(i64 l, f64 r);
  f64 add(f64 l, i64 r);

  object_ref add(object_ref const l, i64 r);
  object_ref add(i64 l, object_ref const r);
  i64 add(i64 l, i64 r);

  obj::ratio_ref add(obj::ratio_ref const l, obj::ratio_ref const r);
  object_ref add(obj::ratio_ref const l, obj::integer_ref const r);
  obj::ratio_ref add(obj::integer_ref const l, obj::ratio_ref const r);

  object_ref promoting_add(object_ref const l, object_ref const r);

  object_ref sub(object_ref const l, object_ref const r);
  object_ref sub(obj::integer_ref const l, object_ref const r);
  object_ref sub(object_ref const l, obj::integer_ref const r);
  object_ref sub(obj::big_decimal_ref const l, object_ref const r);
  object_ref sub(object_ref const l, obj::big_decimal_ref const r);
  i64 sub(obj::integer_ref const l, obj::integer_ref const r);
  f64 sub(obj::real_ref const l, obj::real_ref const r);
  object_ref sub(obj::real_ref const l, object_ref const r);
  object_ref sub(object_ref const l, obj::real_ref const r);
  f64 sub(obj::real_ref const l, obj::integer_ref const r);
  f64 sub(obj::integer_ref const l, obj::real_ref const r);

  object_ref sub(object_ref const l, f64 r);
  object_ref sub(f64 l, object_ref const r);
  f64 sub(f64 l, f64 r);

  f64 sub(i64 l, f64 r);
  f64 sub(f64 l, i64 r);

  object_ref sub(object_ref const l, i64 r);
  object_ref sub(i64 l, object_ref const r);
  i64 sub(i64 l, i64 r);

  object_ref promoting_sub(object_ref const l, object_ref const r);

  object_ref div(object_ref const l, object_ref const r);
  object_ref div(obj::integer_ref const l, object_ref const r);
  object_ref div(object_ref const l, obj::integer_ref const r);
  object_ref div(obj::big_decimal_ref const l, object_ref const r);
  object_ref div(object_ref const l, obj::big_decimal_ref const r);
  i64 div(obj::integer_ref const l, obj::integer_ref const r);
  f64 div(obj::real_ref const l, obj::real_ref const r);
  object_ref div(obj::real_ref const l, object_ref const r);
  object_ref div(object_ref const l, obj::real_ref const r);
  f64 div(obj::real_ref const l, obj::integer_ref const r);
  f64 div(obj::integer_ref const l, obj::real_ref const r);

  object_ref div(object_ref const l, f64 r);
  object_ref div(f64 l, object_ref const r);
  f64 div(f64 l, f64 r);

  f64 div(i64 l, f64 r);
  f64 div(f64 l, i64 r);

  object_ref div(object_ref const l, i64 r);
  object_ref div(i64 l, object_ref const r);
  i64 div(i64 l, i64 r);

  object_ref mul(object_ref const l, object_ref const r);
  object_ref mul(obj::integer_ref const l, object_ref const r);
  object_ref mul(object_ref const l, obj::integer_ref const r);
  object_ref mul(obj::big_decimal_ref const l, object_ref const r);
  object_ref mul(object_ref const l, obj::big_decimal_ref const r);
  i64 mul(obj::integer_ref const l, obj::integer_ref const r);
  f64 mul(obj::real_ref const l, obj::real_ref const r);
  object_ref mul(obj::real_ref const l, object_ref const r);
  object_ref mul(object_ref const l, obj::real_ref const r);
  f64 mul(obj::real_ref const l, obj::integer_ref const r);
  f64 mul(obj::integer_ref const l, obj::real_ref const r);

  object_ref mul(object_ref const l, f64 r);
  object_ref mul(f64 l, object_ref const r);
  f64 mul(f64 l, f64 r);

  f64 mul(i64 l, f64 r);
  f64 mul(f64 l, i64 r);

  object_ref mul(object_ref const l, i64 r);
  object_ref mul(i64 l, object_ref const r);
  i64 mul(i64 l, i64 r);

  object_ref promoting_mul(object_ref const l, object_ref const r);

  bool lt(object_ref const l, object_ref const r);
  bool lt(obj::integer_ref const l, object_ref const r);
  bool lt(object_ref const l, obj::integer_ref const r);
  bool lt(obj::integer_ref const l, obj::integer_ref const r);
  bool lt(obj::real_ref const l, obj::real_ref const r);
  bool lt(obj::real_ref const l, object_ref const r);
  bool lt(object_ref const l, obj::real_ref const r);
  bool lt(obj::real_ref const l, obj::integer_ref const r);
  bool lt(obj::integer_ref const l, obj::real_ref const r);

  bool lt(object_ref const l, f64 r);
  bool lt(f64 l, object_ref const r);
  bool lt(f64 l, f64 r);

  bool lt(i64 l, f64 r);
  bool lt(f64 l, i64 r);

  bool lt(object_ref const l, i64 r);
  bool lt(i64 l, object_ref const r);
  bool lt(i64 l, i64 r);

  bool lte(object_ref const l, object_ref const r);
  bool lte(obj::integer_ref const l, object_ref const r);
  bool lte(object_ref const l, obj::integer_ref const r);
  bool lte(obj::integer_ref const l, obj::integer_ref const r);
  bool lte(obj::real_ref const l, obj::real_ref const r);
  bool lte(obj::real_ref const l, object_ref const r);
  bool lte(object_ref const l, obj::real_ref const r);
  bool lte(obj::real_ref const l, obj::integer_ref const r);
  bool lte(obj::integer_ref const l, obj::real_ref const r);

  bool lte(object_ref const l, f64 r);
  bool lte(f64 l, object_ref const r);
  bool lte(f64 l, f64 r);

  bool lte(i64 l, f64 r);
  bool lte(f64 l, i64 r);

  bool lte(object_ref const l, i64 r);
  bool lte(i64 l, object_ref const r);
  bool lte(i64 l, i64 r);

  object_ref min(object_ref const l, object_ref const r);
  object_ref min(obj::integer_ref const l, object_ref const r);
  object_ref min(object_ref const l, obj::integer_ref const r);
  i64 min(obj::integer_ref const l, obj::integer_ref const r);
  f64 min(obj::real_ref const l, obj::real_ref const r);
  f64 min(obj::real_ref const l, object_ref const r);
  f64 min(object_ref const l, obj::real_ref const r);
  f64 min(obj::real_ref const l, obj::integer_ref const r);
  f64 min(obj::integer_ref const l, obj::real_ref const r);

  f64 min(object_ref const l, f64 r);
  f64 min(f64 l, object_ref const r);
  f64 min(f64 l, f64 r);

  f64 min(i64 l, f64 r);
  f64 min(f64 l, i64 r);

  object_ref min(object_ref const l, i64 r);
  object_ref min(i64 l, object_ref const r);
  i64 min(i64 l, i64 r);

  object_ref max(object_ref const l, object_ref const r);
  object_ref max(obj::integer_ref const l, object_ref const r);
  object_ref max(object_ref const l, obj::integer_ref const r);
  i64 max(obj::integer_ref const l, obj::integer_ref const r);
  f64 max(obj::real_ref const l, obj::real_ref const r);
  f64 max(obj::real_ref const l, object_ref const r);
  f64 max(object_ref const l, obj::real_ref const r);
  f64 max(obj::real_ref const l, obj::integer_ref const r);
  f64 max(obj::integer_ref const l, obj::real_ref const r);

  object_ref max(object_ref const l, f64 r);
  object_ref max(f64 l, object_ref const r);
  f64 max(f64 l, f64 r);

  f64 max(i64 l, f64 r);
  f64 max(f64 l, i64 r);

  object_ref max(object_ref const l, i64 r);
  object_ref max(i64 l, object_ref const r);
  i64 max(i64 l, i64 r);

  object_ref abs(object_ref const l);
  i64 abs(obj::integer_ref const l);
  f64 abs(obj::real_ref const l);
  i64 abs(i64 l);
  f64 abs(f64 l);

  f64 tan(object_ref const l);

  f64 sqrt(object_ref const l);
  f64 sqrt(obj::integer_ref const l);
  f64 sqrt(obj::real_ref const l);
  f64 sqrt(i64 l);
  f64 sqrt(f64 l);

  f64 pow(object_ref const l, object_ref const r);
  f64 pow(obj::integer_ref const l, object_ref const r);
  f64 pow(object_ref const l, obj::integer_ref const r);
  f64 pow(obj::integer_ref const l, obj::integer_ref const r);
  f64 pow(obj::real_ref const l, obj::real_ref const r);
  f64 pow(obj::real_ref const l, object_ref const r);
  f64 pow(object_ref const l, obj::real_ref const r);
  f64 pow(obj::real_ref const l, obj::integer_ref const r);
  f64 pow(obj::integer_ref const l, obj::real_ref const r);

  object_ref pow(object_ref const l, f64 r);
  object_ref pow(f64 l, object_ref const r);
  f64 pow(f64 l, f64 r);

  f64 pow(i64 l, f64 r);
  f64 pow(f64 l, i64 r);

  f64 pow(object_ref const l, i64 r);
  f64 pow(i64 l, object_ref const r);
  f64 pow(i64 l, i64 r);

  object_ref rem(object_ref const l, object_ref const r);
  object_ref quot(object_ref const l, object_ref const r);
  object_ref inc(object_ref const l);
  object_ref promoting_inc(object_ref const l);
  object_ref dec(object_ref const l);
  object_ref promoting_dec(object_ref const l);

  bool is_zero(object_ref const l);
  bool is_pos(object_ref const l);
  bool is_neg(object_ref const l);
  bool is_even(object_ref const l);
  bool is_odd(object_ref const l);

  bool is_equiv(object_ref const l, object_ref const r);

  i64 bit_not(object_ref const l);
  i64 bit_and(object_ref const l, object_ref const r);
  i64 bit_or(object_ref const l, object_ref const r);
  i64 bit_xor(object_ref const l, object_ref const r);
  i64 bit_and_not(object_ref const l, object_ref const r);
  i64 bit_clear(object_ref const l, object_ref const r);
  i64 bit_set(object_ref const l, object_ref const r);
  i64 bit_flip(object_ref const l, object_ref const r);
  bool bit_test(object_ref const l, object_ref const r);
  i64 bit_shift_left(object_ref const l, object_ref const r);
  i64 bit_shift_right(object_ref const l, object_ref const r);
  i64 bit_unsigned_shift_right(object_ref const l, object_ref const r);

  f64 rand();

  native_big_integer numerator(object_ref const o);
  native_big_integer denominator(object_ref const o);

  i64 to_int(object_ref const l);
  i64 to_int(obj::integer_ref const l);
  i64 to_int(obj::real_ref const l);
  i64 to_int(i64 l);
  i64 to_int(f64 l);

  f64 to_real(object_ref const o);

  bool is_number(object_ref const o);
  object_ref number(object_ref const o);

  bool is_integer(object_ref const o);
  bool is_real(object_ref const o);
  bool is_ratio(object_ref const o);
  bool is_boolean(object_ref const o);
  bool is_nan(object_ref const o);
  bool is_infinite(object_ref const o);

  i64 parse_long(object_ref const o);
  f64 parse_double(object_ref const o);

  bool is_big_integer(object_ref const o);
  obj::big_integer_ref to_big_integer(object_ref const o);

  bool is_big_decimal(object_ref const o);
  obj::big_decimal_ref to_big_decimal(object_ref const o);
}
