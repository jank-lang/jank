#include <iostream>
#include <random>
#include <cmath>

#include <fmt/compile.h>

#include <jank/runtime/obj/number.hpp>

namespace jank::runtime::obj
{
  /***** boolean *****/
  boolean::boolean(native_bool const d)
    : data{ d }
  { }

  native_bool boolean::equal(object const &o) const
  {
    auto const *b(o.as_boolean());
    if(!b)
    { return false; }

    return data == b->data;
  }

  void to_string_impl(bool const data, fmt::memory_buffer &buff)
  { format_to(std::back_inserter(buff), FMT_COMPILE("{}"), data ? "true" : "false"); }
  void boolean::to_string(fmt::memory_buffer &buff) const
  { return to_string_impl(data, buff); }
  native_string boolean::to_string() const
  {
    fmt::memory_buffer buff;
    to_string_impl(data, buff);
    return native_string{ buff.data(), buff.size() };
  }
  native_integer boolean::to_hash() const
  { return data ? 1 : 0; }
  boolean const* boolean::as_boolean() const
  { return this; }

  /***** integer *****/
  integer::integer(native_integer const d)
    : data{ d }
  { }

  native_box<integer> integer::create(native_integer const &n)
  { return jank::make_box<integer>(n); }
  native_bool integer::equal(object const &o) const
  {
    auto const *i(o.as_integer());
    if(!i)
    { return false; }

    return data == i->data;
  }
  native_string integer::to_string() const
  { return fmt::format(FMT_COMPILE("{}"), data); }
  void integer::to_string(fmt::memory_buffer &buff) const
  { fmt::format_to(std::back_inserter(buff), FMT_COMPILE("{}"), data); }
  native_integer integer::to_hash() const
  { return data; }
  native_integer integer::get_integer() const
  { return data; }
  native_real integer::get_real() const
  { return data; }
  integer const* integer::as_integer() const
  { return this; }
  number const* integer::as_number() const
  { return this; }

  /***** real *****/
  real::real(native_real const d)
    : data{ d }
  { }

  native_bool real::equal(object const &o) const
  {
    auto const *r(o.as_real());
    if(!r)
    { return false; }

    std::hash<native_real> hasher{};
    return hasher(data) == hasher(r->data);
  }
  native_string real::to_string() const
  { return fmt::format(FMT_COMPILE("{}"), data); }
  void real::to_string(fmt::memory_buffer &buff) const
  { fmt::format_to(std::back_inserter(buff), FMT_COMPILE("{}"), data); }
  native_integer real::to_hash() const
  { return static_cast<native_integer>(data); }
  native_integer real::get_integer() const
  { return static_cast<native_integer>(data); }
  native_real real::get_real() const
  { return data; }
  real const* real::as_real() const
  { return this; }
  number const* real::as_number() const
  { return this; }

  struct integer_ops : number_ops
  {
    number_ops const& combine(number_ops const &l) const override
    { return l.with(*this); }
    number_ops const& with(integer_ops const&) const override;
    number_ops const& with(real_ops const&) const override;

    object_ptr add() const override
    { return jank::make_box<integer>(left + right); }
    object_ptr subtract() const override
    { return jank::make_box<integer>(left - right); }
    object_ptr multiply() const override
    { return jank::make_box<integer>(left * right); }
    object_ptr divide() const override
    { return jank::make_box<integer>(left / right); }
    object_ptr remainder() const override
    { return jank::make_box<integer>(left % right); }
    object_ptr inc() const override
    { return jank::make_box<integer>(left + 1); }
    object_ptr dec() const override
    { return jank::make_box<integer>(left - 1); }
    object_ptr negate() const override
    { return jank::make_box<integer>(-left); }
    object_ptr abs() const override
    { return jank::make_box<integer>(std::labs(left)); }
    object_ptr min() const override
    { return jank::make_box<integer>(std::min(left, right)); }
    object_ptr max() const override
    { return jank::make_box<integer>(std::max(left, right)); }
    native_bool lt() const override
    { return left < right; }
    native_bool lte() const override
    { return left <= right; }
    native_bool gte() const override
    { return left >= right; }
    native_bool equal() const override
    { return left == right; }
    native_bool is_positive() const override
    { return left > 0; }
    native_bool is_negative() const override
    { return left < 0; }
    native_bool is_zero() const override
    { return left == 0; }

    native_integer left{}, right{};
  };

  struct real_ops : number_ops
  {
    number_ops const& combine(number_ops const &l) const override
    { return l.with(*this); }
    number_ops const& with(integer_ops const&) const override;
    number_ops const& with(real_ops const&) const override;

    object_ptr add() const override
    { return jank::make_box<real>(left + right); }
    object_ptr subtract() const override
    { return jank::make_box<real>(left - right); }
    object_ptr multiply() const override
    { return jank::make_box<real>(left * right); }
    object_ptr divide() const override
    { return jank::make_box<real>(left / right); }
    object_ptr remainder() const override
    { return jank::make_box<real>(std::fmod(left, right)); }
    object_ptr inc() const override
    { return jank::make_box<real>(left + 1); }
    object_ptr dec() const override
    { return jank::make_box<real>(right + 1); }
    object_ptr negate() const override
    { return jank::make_box<real>(-left); }
    object_ptr abs() const override
    { return jank::make_box<real>(std::fabs(left)); }
    object_ptr min() const override
    { return jank::make_box<real>(std::min(left, right)); }
    object_ptr max() const override
    { return jank::make_box<real>(std::max(left, right)); }
    native_bool lt() const override
    { return left < right; }
    native_bool lte() const override
    { return left <= right; }
    native_bool gte() const override
    { return left >= right; }
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfloat-equal"
    native_bool equal() const override
    { return left == right; }
#pragma clang diagnostic pop
    native_bool is_positive() const override
    { return left > 0; }
    native_bool is_negative() const override
    { return left < 0; }
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfloat-equal"
    native_bool is_zero() const override
    { return left == 0; }
#pragma clang diagnostic pop

    native_real left{}, right{};
  };

  // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables): These are thread-local.
  static thread_local integer_ops i_ops;
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables): These are thread-local.
  static thread_local real_ops r_ops;

  number_ops const& integer_ops::with(integer_ops const &) const
  { return i_ops; }
  number_ops const& integer_ops::with(real_ops const &) const
  {
    r_ops.left = left;
    return r_ops;
  }

  number_ops const& real_ops::with(integer_ops const &r) const
  {
    r_ops.right = r.right;
    return r_ops;
  }
  number_ops const& real_ops::with(real_ops const &) const
  { return r_ops; }

  number_ops& left_ops(object_ptr const n)
  {
    if(auto const * const i = n->as_integer())
    {
      i_ops.left = i->data;
      return i_ops;
    }
    if(auto const * const r = n->as_real())
    {
      r_ops.left = r->data;
      return r_ops;
    }

    /* TODO: Exception type. */
    throw native_string{ "(left_ops) not a number: " } + n->to_string();
  }

  number_ops& left_ops(obj::integer_ptr const n)
  {
    i_ops.left = n->data;
    return i_ops;
  }

  number_ops& left_ops(obj::real_ptr const n)
  {
    r_ops.left = n->data;
    return r_ops;
  }

  number_ops& right_ops(object_ptr const n)
  {
    if(auto const * const i = n->as_integer())
    {
      i_ops.right = i->data;
      return i_ops;
    }
    if(auto const * const r = n->as_real())
    {
      r_ops.right = r->data;
      return r_ops;
    }

    /* TODO: Exception type. */
    throw native_string{ "(right_ops) not a number: " } + n->to_string();
  }

  number_ops& right_ops(obj::integer_ptr const n)
  {
    i_ops.right = n->data;
    return i_ops;
  }

  number_ops& right_ops(obj::real_ptr const n)
  {
    r_ops.right = n->data;
    return r_ops;
  }

  object_ptr add(object_ptr const l, object_ptr const r)
  { return right_ops(r).combine(left_ops(l)).add(); }
  object_ptr add(integer_ptr const l, object_ptr const r)
  { return right_ops(r).combine(left_ops(l)).add(); }
  object_ptr add(object_ptr const l, integer_ptr const r)
  { return right_ops(r).combine(left_ops(l)).add(); }
  integer_ptr add(integer_ptr const l, integer_ptr const r)
  { return jank::make_box<integer>(l->data + r->data); }
  real_ptr add(real_ptr const l, real_ptr const r)
  { return jank::make_box<real>(l->data + r->data); }
  real_ptr add(real_ptr const l, object_ptr const r)
  { return static_cast<real_ptr>(right_ops(r).combine(left_ops(l)).add()); }
  real_ptr add(object_ptr const l, real_ptr const r)
  { return static_cast<real_ptr>(right_ops(r).combine(left_ops(l)).add()); }
  real_ptr add(real_ptr const l, integer_ptr const r)
  { return jank::make_box<real>(l->data + r->data); }
  real_ptr add(integer_ptr const l, real_ptr const r)
  { return jank::make_box<real>(l->data + r->data); }

  object_ptr sub(object_ptr const l, object_ptr const r)
  { return right_ops(r).combine(left_ops(l)).subtract(); }
  object_ptr sub(integer_ptr const l, object_ptr const r)
  { return right_ops(r).combine(left_ops(l)).subtract(); }
  object_ptr sub(object_ptr const l, integer_ptr const r)
  { return right_ops(r).combine(left_ops(l)).subtract(); }
  integer_ptr sub(integer_ptr const l, integer_ptr const r)
  { return jank::make_box<integer>(l->data - r->data); }
  real_ptr sub(real_ptr const l, real_ptr const r)
  { return jank::make_box<real>(l->data - r->data); }
  real_ptr sub(real_ptr const l, object_ptr const r)
  { return static_cast<real_ptr>(right_ops(r).combine(left_ops(l)).subtract()); }
  real_ptr sub(object_ptr const l, real_ptr const r)
  { return static_cast<real_ptr>(right_ops(r).combine(left_ops(l)).subtract()); }
  real_ptr sub(real_ptr const l, integer_ptr const r)
  { return jank::make_box<real>(l->data - r->data); }
  real_ptr sub(integer_ptr const l, real_ptr const r)
  { return jank::make_box<real>(l->data - r->data); }

  object_ptr div(object_ptr const l, object_ptr const r)
  { return right_ops(r).combine(left_ops(l)).divide(); }
  object_ptr div(integer_ptr const l, object_ptr const r)
  { return right_ops(r).combine(left_ops(l)).divide(); }
  object_ptr div(object_ptr const l, integer_ptr const r)
  { return right_ops(r).combine(left_ops(l)).divide(); }
  integer_ptr div(integer_ptr const l, integer_ptr const r)
  { return jank::make_box<integer>(l->data / r->data); }
  real_ptr div(real_ptr const l, real_ptr const r)
  { return jank::make_box<real>(l->data / r->data); }
  real_ptr div(real_ptr const l, object_ptr const r)
  { return static_cast<real_ptr>(right_ops(r).combine(left_ops(l)).divide()); }
  real_ptr div(object_ptr const l, real_ptr const r)
  { return static_cast<real_ptr>(right_ops(r).combine(left_ops(l)).divide()); }
  real_ptr div(real_ptr const l, integer_ptr const r)
  { return jank::make_box<real>(l->data / r->data); }
  real_ptr div(integer_ptr const l, real_ptr const r)
  { return jank::make_box<real>(l->data / r->data); }

  object_ptr mul(object_ptr const l, object_ptr const r)
  { return right_ops(r).combine(left_ops(l)).multiply(); }
  object_ptr mul(integer_ptr const l, object_ptr const r)
  { return right_ops(r).combine(left_ops(l)).multiply(); }
  object_ptr mul(object_ptr const l, integer_ptr const r)
  { return right_ops(r).combine(left_ops(l)).multiply(); }
  integer_ptr mul(integer_ptr const l, integer_ptr const r)
  { return jank::make_box<integer>(l->data * r->data); }
  real_ptr mul(real_ptr const l, real_ptr const r)
  { return jank::make_box<real>(l->data * r->data); }
  real_ptr mul(real_ptr const l, object_ptr const r)
  { return static_cast<real_ptr>(right_ops(r).combine(left_ops(l)).multiply()); }
  real_ptr mul(object_ptr const l, real_ptr const r)
  { return static_cast<real_ptr>(right_ops(r).combine(left_ops(l)).multiply()); }
  real_ptr mul(real_ptr const l, integer_ptr const r)
  { return jank::make_box<real>(l->data * r->data); }
  real_ptr mul(integer_ptr const l, real_ptr const r)
  { return jank::make_box<real>(l->data * r->data); }

  object_ptr rem(object_ptr const l, object_ptr const r)
  { return right_ops(r).combine(left_ops(l)).remainder(); }

  object_ptr rand()
  {
    static std::mt19937 gen;
    static std::uniform_real_distribution<native_real> dis(0.0, 1.0);
    return jank::make_box<real>(dis(gen));
  }

  bool lt(object_ptr const l, object_ptr const r)
  { return right_ops(r).combine(left_ops(l)).lt(); }
  bool lt(integer_ptr const l, object_ptr const r)
  { return right_ops(r).combine(left_ops(l)).lt(); }
  bool lt(object_ptr const l, integer_ptr const r)
  { return right_ops(r).combine(left_ops(l)).lt(); }
  bool lt(integer_ptr const l, integer_ptr const r)
  { return l->data < r->data; }
  bool lt(real_ptr const l, real_ptr const r)
  { return l->data < r->data; }
  bool lt(real_ptr const l, object_ptr const r)
  { return right_ops(r).combine(left_ops(l)).lt(); }
  bool lt(object_ptr const l, real_ptr const r)
  { return right_ops(r).combine(left_ops(l)).lt(); }
  bool lt(real_ptr const l, integer_ptr const r)
  { return right_ops(r).combine(left_ops(l)).lt(); }
  bool lt(integer_ptr const l, real_ptr const r)
  { return right_ops(r).combine(left_ops(l)).lt(); }

  bool lte(object_ptr const l, object_ptr const r)
  { return right_ops(r).combine(left_ops(l)).lte(); }
  bool lte(integer_ptr const l, object_ptr const r)
  { return right_ops(r).combine(left_ops(l)).lte(); }
  bool lte(object_ptr const l, integer_ptr const r)
  { return right_ops(r).combine(left_ops(l)).lte(); }
  bool lte(integer_ptr const l, integer_ptr const r)
  { return l->data <= r->data; }
  bool lte(real_ptr const l, real_ptr const r)
  { return l->data <= r->data; }
  bool lte(real_ptr const l, object_ptr const r)
  { return right_ops(r).combine(left_ops(l)).lte(); }
  bool lte(object_ptr const l, real_ptr const r)
  { return right_ops(r).combine(left_ops(l)).lte(); }
  bool lte(real_ptr const l, integer_ptr const r)
  { return right_ops(r).combine(left_ops(l)).lte(); }
  bool lte(integer_ptr const l, real_ptr const r)
  { return right_ops(r).combine(left_ops(l)).lte(); }

  object_ptr min(object_ptr const l, object_ptr const r)
  { return right_ops(r).combine(left_ops(l)).min(); }
  object_ptr min(integer_ptr const l, object_ptr const r)
  { return right_ops(r).combine(left_ops(l)).min(); }
  object_ptr min(object_ptr const l, integer_ptr const r)
  { return right_ops(r).combine(left_ops(l)).min(); }
  integer_ptr min(integer_ptr const l, integer_ptr const r)
  { return jank::make_box<integer>(std::min(l->data, r->data)); }
  real_ptr min(real_ptr const l, real_ptr const r)
  { return jank::make_box<real>(std::min(l->data, r->data)); }
  real_ptr min(real_ptr const l, object_ptr const r)
  { return static_cast<real_ptr>(right_ops(r).combine(left_ops(l)).min()); }
  real_ptr min(object_ptr const l, real_ptr const r)
  { return static_cast<real_ptr>(right_ops(r).combine(left_ops(l)).min()); }
  real_ptr min(real_ptr const l, integer_ptr const r)
  { return jank::make_box<real>(std::min(l->data, static_cast<native_real>(r->data))); }
  real_ptr min(integer_ptr const l, real_ptr const r)
  { return jank::make_box<real>(std::min(static_cast<native_real>(l->data), r->data)); }

  object_ptr max(object_ptr const l, object_ptr const r)
  { return right_ops(r).combine(left_ops(l)).max(); }
  object_ptr max(integer_ptr const l, object_ptr const r)
  { return right_ops(r).combine(left_ops(l)).max(); }
  object_ptr max(object_ptr const l, integer_ptr const r)
  { return right_ops(r).combine(left_ops(l)).max(); }
  integer_ptr max(integer_ptr const l, integer_ptr const r)
  { return jank::make_box<integer>(std::max(l->data, r->data)); }
  real_ptr max(real_ptr const l, real_ptr const r)
  { return jank::make_box<real>(std::max(l->data, r->data)); }
  real_ptr max(real_ptr const l, object_ptr const r)
  { return static_cast<real_ptr>(right_ops(r).combine(left_ops(l)).max()); }
  real_ptr max(object_ptr const l, real_ptr const r)
  { return static_cast<real_ptr>(right_ops(r).combine(left_ops(l)).max()); }
  real_ptr max(real_ptr const l, integer_ptr const r)
  { return jank::make_box<real>(std::max(l->data, static_cast<native_real>(r->data))); }
  real_ptr max(integer_ptr const l, real_ptr const r)
  { return jank::make_box<real>(std::max(static_cast<native_real>(l->data), r->data)); }

  object_ptr abs(object_ptr const l)
  { return left_ops(l).abs(); }
  integer_ptr abs(integer_ptr const l)
  { return jank::make_box<integer>(std::abs(l->data)); }
  real_ptr abs(real_ptr const l)
  { return jank::make_box<real>(std::fabs(l->data)); }
}
