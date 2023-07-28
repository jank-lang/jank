#pragma once

#include <jank/runtime/obj/number.hpp>

namespace jank::runtime
{
  struct integer_ops;
  struct real_ops;
  struct number_ops
  {
    virtual ~number_ops() = default;

    virtual number_ops const& combine(number_ops const&) const = 0;
    virtual number_ops const& with(integer_ops const&) const = 0;
    virtual number_ops const& with(real_ops const&) const = 0;

    virtual object_ptr add() const = 0;
    virtual native_real add_real() const = 0;
    virtual object_ptr subtract() const = 0;
    virtual native_real sub_real() const = 0;
    virtual object_ptr multiply() const = 0;
    virtual native_real mul_real() const = 0;
    virtual object_ptr divide() const = 0;
    virtual native_real div_real() const = 0;
    virtual object_ptr remainder() const = 0;
    virtual object_ptr inc() const = 0;
    virtual object_ptr dec() const = 0;
    virtual object_ptr negate() const = 0;
    virtual object_ptr abs() const = 0;
    virtual object_ptr min() const = 0;
    virtual native_real min_real() const = 0;
    virtual object_ptr max() const = 0;
    virtual native_real max_real() const = 0;
    virtual native_real pow() const = 0;
    virtual native_bool lt() const = 0;
    virtual native_bool lte() const = 0;
    virtual native_bool gte() const = 0;
    virtual native_bool equal() const = 0;
    virtual native_bool is_positive() const = 0;
    virtual native_bool is_negative() const = 0;
    virtual native_bool is_zero() const = 0;
  };

  number_ops& left_ops(object_ptr n);
  number_ops& right_ops(object_ptr n);

  object_ptr add(object_ptr l, object_ptr r);
  object_ptr add(obj::integer_ptr l, object_ptr r);
  object_ptr add(object_ptr l, obj::integer_ptr r);
  native_integer add(obj::integer_ptr l, obj::integer_ptr r);
  native_real add(obj::real_ptr l, obj::real_ptr r);
  native_real add(obj::real_ptr l, object_ptr r);
  native_real add(object_ptr l, obj::real_ptr r);
  native_real add(obj::real_ptr l, obj::integer_ptr r);
  native_real add(obj::integer_ptr l, obj::real_ptr r);

  native_real add(object_ptr l, native_real r);
  native_real add(native_real l, object_ptr r);
  native_real add(native_real l, native_real r);

  native_real add(native_integer l, native_real r);
  native_real add(native_real l, native_integer r);

  object_ptr add(object_ptr l, native_integer r);
  object_ptr add(native_integer l, object_ptr r);
  native_integer add(native_integer l, native_integer r);

  object_ptr sub(object_ptr l, object_ptr r);
  object_ptr sub(obj::integer_ptr l, object_ptr r);
  object_ptr sub(object_ptr l, obj::integer_ptr r);
  native_integer sub(obj::integer_ptr l, obj::integer_ptr r);
  native_real sub(obj::real_ptr l, obj::real_ptr r);
  native_real sub(obj::real_ptr l, object_ptr r);
  native_real sub(object_ptr l, obj::real_ptr r);
  native_real sub(obj::real_ptr l, obj::integer_ptr r);
  native_real sub(obj::integer_ptr l, obj::real_ptr r);

  native_real sub(object_ptr l, native_real r);
  native_real sub(native_real l, object_ptr r);
  native_real sub(native_real l, native_real r);

  native_real sub(native_integer l, native_real r);
  native_real sub(native_real l, native_integer r);

  object_ptr sub(object_ptr l, native_integer r);
  object_ptr sub(native_integer l, object_ptr r);
  native_integer sub(native_integer l, native_integer r);

  object_ptr div(object_ptr l, object_ptr r);
  object_ptr div(obj::integer_ptr l, object_ptr r);
  object_ptr div(object_ptr l, obj::integer_ptr r);
  native_integer div(obj::integer_ptr l, obj::integer_ptr r);
  native_real div(obj::real_ptr l, obj::real_ptr r);
  native_real div(obj::real_ptr l, object_ptr r);
  native_real div(object_ptr l, obj::real_ptr r);
  native_real div(obj::real_ptr l, obj::integer_ptr r);
  native_real div(obj::integer_ptr l, obj::real_ptr r);

  native_real div(object_ptr l, native_real r);
  native_real div(native_real l, object_ptr r);
  native_real div(native_real l, native_real r);

  native_real div(native_integer l, native_real r);
  native_real div(native_real l, native_integer r);

  object_ptr div(object_ptr l, native_integer r);
  object_ptr div(native_integer l, object_ptr r);
  native_integer div(native_integer l, native_integer r);

  object_ptr mul(object_ptr l, object_ptr r);
  object_ptr mul(obj::integer_ptr l, object_ptr r);
  object_ptr mul(object_ptr l, obj::integer_ptr r);
  native_integer mul(obj::integer_ptr l, obj::integer_ptr r);
  native_real mul(obj::real_ptr l, obj::real_ptr r);
  native_real mul(obj::real_ptr l, object_ptr r);
  native_real mul(object_ptr l, obj::real_ptr r);
  native_real mul(obj::real_ptr l, obj::integer_ptr r);
  native_real mul(obj::integer_ptr l, obj::real_ptr r);

  native_real mul(object_ptr l, native_real r);
  native_real mul(native_real l, object_ptr r);
  native_real mul(native_real l, native_real r);

  native_real mul(native_integer l, native_real r);
  native_real mul(native_real l, native_integer r);

  object_ptr mul(object_ptr l, native_integer r);
  object_ptr mul(native_integer l, object_ptr r);
  native_integer mul(native_integer l, native_integer r);

  bool lt(object_ptr l, object_ptr r);
  bool lt(obj::integer_ptr l, object_ptr r);
  bool lt(object_ptr l, obj::integer_ptr r);
  bool lt(obj::integer_ptr const l, obj::integer_ptr const r);
  bool lt(obj::real_ptr const l, obj::real_ptr const r);
  bool lt(obj::real_ptr l, object_ptr r);
  bool lt(object_ptr l, obj::real_ptr r);
  bool lt(obj::real_ptr l, obj::integer_ptr r);
  bool lt(obj::integer_ptr l, obj::real_ptr r);

  bool lt(object_ptr l, native_real r);
  bool lt(native_real l, object_ptr r);
  bool lt(native_real l, native_real r);

  bool lt(native_integer l, native_real r);
  bool lt(native_real l, native_integer r);

  bool lt(object_ptr l, native_integer r);
  bool lt(native_integer l, object_ptr r);
  bool lt(native_integer l, native_integer r);

  bool lte(object_ptr l, object_ptr r);
  bool lte(obj::integer_ptr l, object_ptr r);
  bool lte(object_ptr l, obj::integer_ptr r);
  bool lte(obj::integer_ptr const l, obj::integer_ptr const r);
  bool lte(obj::real_ptr const l, obj::real_ptr const r);
  bool lte(obj::real_ptr l, object_ptr r);
  bool lte(object_ptr l, obj::real_ptr r);
  bool lte(obj::real_ptr l, obj::integer_ptr r);
  bool lte(obj::integer_ptr l, obj::real_ptr r);

  bool lte(object_ptr l, native_real r);
  bool lte(native_real l, object_ptr r);
  bool lte(native_real l, native_real r);

  bool lte(native_integer l, native_real r);
  bool lte(native_real l, native_integer r);

  bool lte(object_ptr l, native_integer r);
  bool lte(native_integer l, object_ptr r);
  bool lte(native_integer l, native_integer r);

  object_ptr min(object_ptr l, object_ptr r);
  object_ptr min(obj::integer_ptr l, object_ptr r);
  object_ptr min(object_ptr l, obj::integer_ptr r);
  native_integer min(obj::integer_ptr l, obj::integer_ptr r);
  native_real min(obj::real_ptr l, obj::real_ptr r);
  native_real min(obj::real_ptr l, object_ptr r);
  native_real min(object_ptr l, obj::real_ptr r);
  native_real min(obj::real_ptr l, obj::integer_ptr r);
  native_real min(obj::integer_ptr l, obj::real_ptr r);

  native_real min(object_ptr l, native_real r);
  native_real min(native_real l, object_ptr r);
  native_real min(native_real l, native_real r);

  native_real min(native_integer l, native_real r);
  native_real min(native_real l, native_integer r);

  object_ptr min(object_ptr l, native_integer r);
  object_ptr min(native_integer l, object_ptr r);
  native_integer min(native_integer l, native_integer r);

  object_ptr max(object_ptr l, object_ptr r);
  object_ptr max(obj::integer_ptr l, object_ptr r);
  object_ptr max(object_ptr l, obj::integer_ptr r);
  native_integer max(obj::integer_ptr l, obj::integer_ptr r);
  native_real max(obj::real_ptr l, obj::real_ptr r);
  native_real max(obj::real_ptr l, object_ptr r);
  native_real max(object_ptr l, obj::real_ptr r);
  native_real max(obj::real_ptr l, obj::integer_ptr r);
  native_real max(obj::integer_ptr l, obj::real_ptr r);

  native_real max(object_ptr l, native_real r);
  native_real max(native_real l, object_ptr r);
  native_real max(native_real l, native_real r);

  native_real max(native_integer l, native_real r);
  native_real max(native_real l, native_integer r);

  object_ptr max(object_ptr l, native_integer r);
  object_ptr max(native_integer l, object_ptr r);
  native_integer max(native_integer l, native_integer r);

  object_ptr abs(object_ptr l);
  native_integer abs(obj::integer_ptr l);
  native_real abs(obj::real_ptr l);
  native_integer abs(native_integer l);
  native_real abs(native_real l);

  native_real sqrt(object_ptr l);
  native_real sqrt(obj::integer_ptr l);
  native_real sqrt(obj::real_ptr l);
  native_real sqrt(native_integer l);
  native_real sqrt(native_real l);

  native_real pow(object_ptr l, object_ptr r);
  native_real pow(obj::integer_ptr l, object_ptr r);
  native_real pow(object_ptr l, obj::integer_ptr r);
  native_real pow(obj::integer_ptr l, obj::integer_ptr r);
  native_real pow(obj::real_ptr l, obj::real_ptr r);
  native_real pow(obj::real_ptr l, object_ptr r);
  native_real pow(object_ptr l, obj::real_ptr r);
  native_real pow(obj::real_ptr l, obj::integer_ptr r);
  native_real pow(obj::integer_ptr l, obj::real_ptr r);

  native_real pow(object_ptr l, native_real r);
  native_real pow(native_real l, object_ptr r);
  native_real pow(native_real l, native_real r);

  native_real pow(native_integer l, native_real r);
  native_real pow(native_real l, native_integer r);

  native_real pow(object_ptr l, native_integer r);
  native_real pow(native_integer l, object_ptr r);
  native_real pow(native_integer l, native_integer r);

  object_ptr rem(object_ptr l, object_ptr r);
  native_real rand();

  native_integer to_int(object_ptr l);
  native_integer to_int(obj::integer_ptr l);
  native_integer to_int(obj::real_ptr l);
  native_integer to_int(native_integer l);
  native_integer to_int(native_real l);
}
