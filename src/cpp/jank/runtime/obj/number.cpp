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

  /***** math ops *****/

  struct integer_ops : number_ops
  {
    number_ops const& combine(number_ops const &l) const override
    { return l.with(*this); }
    number_ops const& with(integer_ops const&) const override;
    number_ops const& with(real_ops const&) const override;

    object_ptr add() const override
    { return jank::make_box<integer>(left + right); }
    native_real add_real() const override
    { return left + right; }
    object_ptr subtract() const override
    { return jank::make_box<integer>(left - right); }
    native_real sub_real() const override
    { return left - right; }
    object_ptr multiply() const override
    { return jank::make_box<integer>(left * right); }
    native_real mul_real() const override
    { return left * right; }
    object_ptr divide() const override
    { return jank::make_box<integer>(left / right); }
    native_real div_real() const override
    { return static_cast<native_real>(left) / right; }
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
    native_real min_real() const override
    { return std::min(left, right); }
    object_ptr max() const override
    { return jank::make_box<integer>(std::max(left, right)); }
    native_real max_real() const override
    { return std::max(left, right); }
    native_real pow() const override
    { return std::pow(left, right); }
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
    native_real add_real() const override
    { return left + right; }
    object_ptr subtract() const override
    { return jank::make_box<real>(left - right); }
    native_real sub_real() const override
    { return left - right; }
    object_ptr multiply() const override
    { return jank::make_box<real>(left * right); }
    native_real mul_real() const override
    { return left * right; }
    object_ptr divide() const override
    { return jank::make_box<real>(left / right); }
    native_real div_real() const override
    { return left / right; }
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
    native_real min_real() const override
    { return std::min(left, right); }
    object_ptr max() const override
    { return jank::make_box<real>(std::max(left, right)); }
    native_real max_real() const override
    { return std::max(left, right); }
    native_real pow() const override
    { return std::pow(left, right); }
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
    throw std::runtime_error{ (native_string{ "(left_ops) not a number: " } + n->to_string()).c_str() };
  }
  integer_ops& left_ops(obj::integer_ptr const n)
  {
    i_ops.left = n->data;
    return i_ops;
  }
  real_ops& left_ops(obj::real_ptr const n)
  {
    r_ops.left = n->data;
    return r_ops;
  }
  integer_ops& left_ops(native_integer const n)
  {
    i_ops.left = n;
    return i_ops;
  }
  real_ops& left_ops(native_real const n)
  {
    r_ops.left = n;
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
    throw std::runtime_error{ (native_string{ "(right_ops) not a number: " } + n->to_string()).c_str() };
  }
  integer_ops& right_ops(obj::integer_ptr const n)
  {
    i_ops.right = n->data;
    return i_ops;
  }
  real_ops& right_ops(obj::real_ptr const n)
  {
    r_ops.right = n->data;
    return r_ops;
  }
  integer_ops& right_ops(native_integer const n)
  {
    i_ops.right = n;
    return i_ops;
  }
  real_ops& right_ops(native_real const n)
  {
    r_ops.right = n;
    return r_ops;
  }

  /* This version of `with` avoids two dynamic dispatches per operation, so it's
   * preferable over `number_ops.combine`. */
  integer_ops const& with(integer_ops const &, integer_ops const &)
  { return i_ops; }
  real_ops const& with(real_ops const &, real_ops const &)
  { return r_ops; }
  real_ops const& with(integer_ops const &, real_ops const &)
  {
    r_ops.left = i_ops.left;
    return r_ops;
  }
  real_ops const& with(real_ops const &, integer_ops const &)
  {
    r_ops.right = i_ops.right;
    return r_ops;
  }
  number_ops const& with(number_ops const &l, number_ops const &r)
  { return r.combine(l); }

  object_ptr add(object_ptr const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).add(); }
  object_ptr add(integer_ptr const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).add(); }
  object_ptr add(object_ptr const l, integer_ptr const r)
  { return with(left_ops(l), right_ops(r)).add(); }
  native_integer add(integer_ptr const l, integer_ptr const r)
  { return l->data + r->data; }
  native_real add(real_ptr const l, real_ptr const r)
  { return l->data + r->data; }
  native_real add(real_ptr const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).add_real(); }
  native_real add(object_ptr const l, real_ptr const r)
  { return with(left_ops(l), right_ops(r)).add_real(); }
  native_real add(real_ptr const l, integer_ptr const r)
  { return l->data + r->data; }
  native_real add(integer_ptr const l, real_ptr const r)
  { return l->data + r->data; }

  native_real add(object_ptr const l, native_real const r)
  { return with(left_ops(l), right_ops(r)).add_real(); }
  native_real add(native_real const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).add_real(); }
  native_real add(native_real const l, native_real const r)
  { return l + r; }

  native_real add(native_integer const l, native_real const r)
  { return l + r; }
  native_real add(native_real const l, native_integer const r)
  { return l + r; }

  object_ptr add(object_ptr const l, native_integer const r)
  { return with(left_ops(l), right_ops(r)).add(); }
  object_ptr add(native_integer const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).add(); }
  native_integer add(native_integer const l, native_integer const r)
  { return l + r; }

  object_ptr sub(object_ptr const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).subtract(); }
  object_ptr sub(integer_ptr const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).subtract(); }
  object_ptr sub(object_ptr const l, integer_ptr const r)
  { return with(left_ops(l), right_ops(r)).subtract(); }
  native_integer sub(integer_ptr const l, integer_ptr const r)
  { return l->data - r->data; }
  native_real sub(real_ptr const l, real_ptr const r)
  { return l->data - r->data; }
  native_real sub(real_ptr const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).sub_real(); }
  native_real sub(object_ptr const l, real_ptr const r)
  { return with(left_ops(l), right_ops(r)).sub_real(); }
  native_real sub(real_ptr const l, integer_ptr const r)
  { return l->data - r->data; }
  native_real sub(integer_ptr const l, real_ptr const r)
  { return l->data - r->data; }

  native_real sub(object_ptr const l, native_real const r)
  { return with(left_ops(l), right_ops(r)).sub_real(); }
  native_real sub(native_real const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).sub_real(); }
  native_real sub(native_real const l, native_real const r)
  { return l - r; }

  native_real sub(native_integer const l, native_real const r)
  { return l - r; }
  native_real sub(native_real const l, native_integer const r)
  { return l - r; }

  object_ptr sub(object_ptr const l, native_integer const r)
  { return with(left_ops(l), right_ops(r)).subtract(); }
  object_ptr sub(native_integer const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).subtract(); }
  native_integer sub(native_integer const l, native_integer const r)
  { return l - r; }

  object_ptr div(object_ptr const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).divide(); }
  object_ptr div(integer_ptr const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).divide(); }
  object_ptr div(object_ptr const l, integer_ptr const r)
  { return with(left_ops(l), right_ops(r)).divide(); }
  native_integer div(integer_ptr const l, integer_ptr const r)
  { return l->data / r->data; }
  native_real div(real_ptr const l, real_ptr const r)
  { return l->data / r->data; }
  native_real div(real_ptr const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).div_real(); }
  native_real div(object_ptr const l, real_ptr const r)
  { return with(left_ops(l), right_ops(r)).div_real(); }
  native_real div(real_ptr const l, integer_ptr const r)
  { return l->data / r->data; }
  native_real div(integer_ptr const l, real_ptr const r)
  { return l->data / r->data; }

  native_real div(object_ptr const l, native_real const r)
  { return with(left_ops(l), right_ops(r)).div_real(); }
  native_real div(native_real const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).div_real(); }
  native_real div(native_real const l, native_real const r)
  { return l / r; }

  native_real div(native_integer const l, native_real const r)
  { return l / r; }
  native_real div(native_real const l, native_integer const r)
  { return l / r; }

  object_ptr div(object_ptr const l, native_integer const r)
  { return with(left_ops(l), right_ops(r)).divide(); }
  object_ptr div(native_integer const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).divide(); }
  native_integer div(native_integer const l, native_integer const r)
  { return l / r; }

  object_ptr mul(object_ptr const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).multiply(); }
  object_ptr mul(integer_ptr const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).multiply(); }
  object_ptr mul(object_ptr const l, integer_ptr const r)
  { return with(left_ops(l), right_ops(r)).multiply(); }
  native_integer mul(integer_ptr const l, integer_ptr const r)
  { return l->data * r->data; }
  native_real mul(real_ptr const l, real_ptr const r)
  { return l->data * r->data; }
  native_real mul(real_ptr const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).mul_real(); }
  native_real mul(object_ptr const l, real_ptr const r)
  { return with(left_ops(l), right_ops(r)).mul_real(); }
  native_real mul(real_ptr const l, integer_ptr const r)
  { return l->data * r->data; }
  native_real mul(integer_ptr const l, real_ptr const r)
  { return l->data * r->data; }

  native_real mul(object_ptr const l, native_real const r)
  { return with(left_ops(l), right_ops(r)).mul_real(); }
  native_real mul(native_real const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).mul_real(); }
  native_real mul(native_real const l, native_real const r)
  { return l * r; }

  native_real mul(native_integer const l, native_real const r)
  { return l * r; }
  native_real mul(native_real const l, native_integer const r)
  { return l * r; }

  object_ptr mul(object_ptr const l, native_integer const r)
  { return with(left_ops(l), right_ops(r)).multiply(); }
  object_ptr mul(native_integer const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).multiply(); }
  native_integer mul(native_integer const l, native_integer const r)
  { return l * r; }

  object_ptr rem(object_ptr const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).remainder(); }

  native_real rand()
  {
    static std::mt19937 gen;
    static std::uniform_real_distribution<native_real> dis(0.0, 1.0);
    return dis(gen);
  }

  bool lt(object_ptr const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).lt(); }
  bool lt(integer_ptr const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).lt(); }
  bool lt(object_ptr const l, integer_ptr const r)
  { return with(left_ops(l), right_ops(r)).lt(); }
  bool lt(integer_ptr const l, integer_ptr const r)
  { return l->data < r->data; }
  bool lt(real_ptr const l, real_ptr const r)
  { return l->data < r->data; }
  bool lt(real_ptr const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).lt(); }
  bool lt(object_ptr const l, real_ptr const r)
  { return with(left_ops(l), right_ops(r)).lt(); }
  bool lt(real_ptr const l, integer_ptr const r)
  { return with(left_ops(l), right_ops(r)).lt(); }
  bool lt(integer_ptr const l, real_ptr const r)
  { return with(left_ops(l), right_ops(r)).lt(); }

  native_bool lt(object_ptr const l, native_real const r)
  { return with(left_ops(l), right_ops(r)).lt(); }
  native_bool lt(native_real const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).lt(); }
  native_bool lt(native_real const l, native_real const r)
  { return l < r; }

  native_bool lt(native_integer const l, native_real const r)
  { return l < r; }
  native_bool lt(native_real const l, native_integer const r)
  { return l < r; }

  native_bool lt(object_ptr const l, native_integer const r)
  { return with(left_ops(l), right_ops(r)).lt(); }
  native_bool lt(native_integer const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).lt(); }
  native_bool lt(native_integer const l, native_integer const r)
  { return l < r; }

  bool lte(object_ptr const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).lte(); }
  bool lte(integer_ptr const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).lte(); }
  bool lte(object_ptr const l, integer_ptr const r)
  { return with(left_ops(l), right_ops(r)).lte(); }
  bool lte(integer_ptr const l, integer_ptr const r)
  { return l->data <= r->data; }
  bool lte(real_ptr const l, real_ptr const r)
  { return l->data <= r->data; }
  bool lte(real_ptr const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).lte(); }
  bool lte(object_ptr const l, real_ptr const r)
  { return with(left_ops(l), right_ops(r)).lte(); }
  bool lte(real_ptr const l, integer_ptr const r)
  { return with(left_ops(l), right_ops(r)).lte(); }
  bool lte(integer_ptr const l, real_ptr const r)
  { return with(left_ops(l), right_ops(r)).lte(); }

  native_bool lte(object_ptr const l, native_real const r)
  { return with(left_ops(l), right_ops(r)).lte(); }
  native_bool lte(native_real const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).lte(); }
  native_bool lte(native_real const l, native_real const r)
  { return l < r; }

  native_bool lte(native_integer const l, native_real const r)
  { return l < r; }
  native_bool lte(native_real const l, native_integer const r)
  { return l < r; }

  native_bool lte(object_ptr const l, native_integer const r)
  { return with(left_ops(l), right_ops(r)).lte(); }
  native_bool lte(native_integer const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).lte(); }
  native_bool lte(native_integer const l, native_integer const r)
  { return l < r; }

  object_ptr min(object_ptr const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).min(); }
  object_ptr min(integer_ptr const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).min(); }
  object_ptr min(object_ptr const l, integer_ptr const r)
  { return with(left_ops(l), right_ops(r)).min(); }
  native_integer min(integer_ptr const l, integer_ptr const r)
  { return std::min(l->data, r->data); }
  native_real min(real_ptr const l, real_ptr const r)
  { return std::min(l->data, r->data); }
  native_real min(real_ptr const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).min_real(); }
  native_real min(object_ptr const l, real_ptr const r)
  { return with(left_ops(l), right_ops(r)).min_real(); }
  native_real min(real_ptr const l, integer_ptr const r)
  { return std::min(l->data, static_cast<native_real>(r->data)); }
  native_real min(integer_ptr const l, real_ptr const r)
  { return std::min(static_cast<native_real>(l->data), r->data); }

  native_real min(object_ptr const l, native_real const r)
  { return with(left_ops(l), right_ops(r)).min_real(); }
  native_real min(native_real const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).min_real(); }
  native_real min(native_real const l, native_real const r)
  { return std::min(l, r); }

  native_real min(native_integer const l, native_real const r)
  { return std::min(static_cast<native_real>(l), r); }
  native_real min(native_real const l, native_integer const r)
  { return std::min(l, static_cast<native_real>(r)); }

  object_ptr min(object_ptr const l, native_integer const r)
  { return with(left_ops(l), right_ops(r)).min(); }
  object_ptr min(native_integer const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).min(); }
  native_integer min(native_integer const l, native_integer const r)
  { return std::min(l, r); }

  object_ptr max(object_ptr const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).max(); }
  object_ptr max(integer_ptr const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).max(); }
  object_ptr max(object_ptr const l, integer_ptr const r)
  { return with(left_ops(l), right_ops(r)).max(); }
  native_integer max(integer_ptr const l, integer_ptr const r)
  { return std::max(l->data, r->data); }
  native_real max(real_ptr const l, real_ptr const r)
  { return std::max(l->data, r->data); }
  native_real max(real_ptr const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).max_real(); }
  native_real max(object_ptr const l, real_ptr const r)
  { return with(left_ops(l), right_ops(r)).max_real(); }
  native_real max(real_ptr const l, integer_ptr const r)
  { return std::max(l->data, static_cast<native_real>(r->data)); }
  native_real max(integer_ptr const l, real_ptr const r)
  { return std::max(static_cast<native_real>(l->data), r->data); }

  native_real max(object_ptr const l, native_real const r)
  { return with(left_ops(l), right_ops(r)).max_real(); }
  native_real max(native_real const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).max_real(); }
  native_real max(native_real const l, native_real const r)
  { return std::max(l, r); }

  native_real max(native_integer const l, native_real const r)
  { return std::max(static_cast<native_real>(l), r); }
  native_real max(native_real const l, native_integer const r)
  { return std::max(l, static_cast<native_real>(r)); }

  object_ptr max(object_ptr const l, native_integer const r)
  { return with(left_ops(l), right_ops(r)).max(); }
  object_ptr max(native_integer const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).max(); }
  native_integer max(native_integer const l, native_integer const r)
  { return std::max(l, r); }

  object_ptr abs(object_ptr const l)
  { return left_ops(l).abs(); }
  native_integer abs(integer_ptr const l)
  { return std::abs(l->data); }
  native_real abs(real_ptr const l)
  { return std::fabs(l->data); }
  native_integer abs(native_integer const l)
  { return std::abs(l); }
  native_real abs(native_real const l)
  { return std::fabs(l); }

  native_real sqrt(object_ptr const l)
  {
    auto const n(l->as_number());
    if(!n)
    { throw std::runtime_error{ fmt::format("not a number: {}", l->to_string()) }; }
    return std::sqrt(n->get_real());
  }
  native_real sqrt(integer_ptr const l)
  { return std::sqrt(l->data); }
  native_real sqrt(real_ptr const l)
  { return std::sqrt(l->data); }
  native_real sqrt(native_integer const l)
  { return std::sqrt(l); }
  native_real sqrt(native_real const l)
  { return std::sqrt(l); }

  native_real pow(object_ptr const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).pow(); }
  native_real pow(integer_ptr const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).pow(); }
  native_real pow(object_ptr const l, integer_ptr const r)
  { return with(left_ops(l), right_ops(r)).pow(); }
  native_real pow(integer_ptr const l, integer_ptr const r)
  { return std::pow(l->data, r->data); }
  native_real pow(real_ptr const l, real_ptr const r)
  { return std::pow(l->data, r->data); }
  native_real pow(real_ptr const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).pow(); }
  native_real pow(object_ptr const l, real_ptr const r)
  { return with(left_ops(l), right_ops(r)).pow(); }
  native_real pow(real_ptr const l, integer_ptr const r)
  { return std::pow(l->data, r->data); }
  native_real pow(integer_ptr const l, real_ptr const r)
  { return std::pow(l->data, r->data); }

  native_real pow(object_ptr const l, native_real const r)
  { return with(left_ops(l), right_ops(r)).pow(); }
  native_real pow(native_real const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).pow(); }
  native_real pow(native_real const l, native_real const r)
  { return std::pow(l, r); }

  native_real pow(native_integer const l, native_real const r)
  { return std::pow(l, r); }
  native_real pow(native_real const l, native_integer const r)
  { return std::pow(l, r); }

  native_real pow(object_ptr const l, native_integer const r)
  { return with(left_ops(l), right_ops(r)).pow(); }
  native_real pow(native_integer const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).pow(); }
  native_real pow(native_integer const l, native_integer const r)
  { return std::pow(l, r); }

  native_integer to_int(object_ptr const l)
  {
    auto const n(l->as_number());
    if(!n)
    { throw std::runtime_error{ fmt::format("not a number: {}", l->to_string()) }; }
    return n->get_integer();
  }
  native_integer to_int(integer_ptr const l)
  { return l->data; }
  native_integer to_int(real_ptr const l)
  { return static_cast<native_integer>(l->data); }
  native_integer to_int(native_integer const l)
  { return l; }
  native_integer to_int(native_real const l)
  { return static_cast<native_integer>(l); }
}
