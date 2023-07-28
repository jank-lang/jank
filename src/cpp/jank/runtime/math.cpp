#include <jank/runtime/math.hpp>
#include <jank/runtime/behavior/numberable.hpp>

namespace jank::runtime
{
  struct integer_ops : number_ops
  {
    number_ops const& combine(number_ops const &l) const final
    { return l.with(*this); }
    number_ops const& with(integer_ops const&) const final;
    number_ops const& with(real_ops const&) const final;

    object_ptr add() const final
    { return jank::make_box<obj::integer>(left + right); }
    native_real add_real() const final
    { return left + right; }
    object_ptr subtract() const final
    { return jank::make_box<obj::integer>(left - right); }
    native_real sub_real() const final
    { return left - right; }
    object_ptr multiply() const final
    { return jank::make_box<obj::integer>(left * right); }
    native_real mul_real() const final
    { return left * right; }
    object_ptr divide() const final
    { return jank::make_box<obj::integer>(left / right); }
    native_real div_real() const final
    { return static_cast<native_real>(left) / right; }
    object_ptr remainder() const final
    { return jank::make_box<obj::integer>(left % right); }
    object_ptr inc() const final
    { return jank::make_box<obj::integer>(left + 1); }
    object_ptr dec() const final
    { return jank::make_box<obj::integer>(left - 1); }
    object_ptr negate() const final
    { return jank::make_box<obj::integer>(-left); }
    object_ptr abs() const final
    { return jank::make_box<obj::integer>(std::labs(left)); }
    object_ptr min() const final
    { return jank::make_box<obj::integer>(std::min(left, right)); }
    native_real min_real() const final
    { return std::min(left, right); }
    object_ptr max() const final
    { return jank::make_box<obj::integer>(std::max(left, right)); }
    native_real max_real() const final
    { return std::max(left, right); }
    native_real pow() const final
    { return std::pow(left, right); }
    native_bool lt() const final
    { return left < right; }
    native_bool lte() const final
    { return left <= right; }
    native_bool gte() const final
    { return left >= right; }
    native_bool equal() const final
    { return left == right; }
    native_bool is_positive() const final
    { return left > 0; }
    native_bool is_negative() const final
    { return left < 0; }
    native_bool is_zero() const final
    { return left == 0; }

    native_integer left{}, right{};
  };

  struct real_ops : number_ops
  {
    number_ops const& combine(number_ops const &l) const final
    { return l.with(*this); }
    number_ops const& with(integer_ops const&) const final;
    number_ops const& with(real_ops const&) const final;

    object_ptr add() const final
    { return jank::make_box<obj::real>(left + right); }
    native_real add_real() const final
    { return left + right; }
    object_ptr subtract() const final
    { return jank::make_box<obj::real>(left - right); }
    native_real sub_real() const final
    { return left - right; }
    object_ptr multiply() const final
    { return jank::make_box<obj::real>(left * right); }
    native_real mul_real() const final
    { return left * right; }
    object_ptr divide() const final
    { return jank::make_box<obj::real>(left / right); }
    native_real div_real() const final
    { return left / right; }
    object_ptr remainder() const final
    { return jank::make_box<obj::real>(std::fmod(left, right)); }
    object_ptr inc() const final
    { return jank::make_box<obj::real>(left + 1); }
    object_ptr dec() const final
    { return jank::make_box<obj::real>(right + 1); }
    object_ptr negate() const final
    { return jank::make_box<obj::real>(-left); }
    object_ptr abs() const final
    { return jank::make_box<obj::real>(std::fabs(left)); }
    object_ptr min() const final
    { return jank::make_box<obj::real>(std::min(left, right)); }
    native_real min_real() const final
    { return std::min(left, right); }
    object_ptr max() const final
    { return jank::make_box<obj::real>(std::max(left, right)); }
    native_real max_real() const final
    { return std::max(left, right); }
    native_real pow() const final
    { return std::pow(left, right); }
    native_bool lt() const final
    { return left < right; }
    native_bool lte() const final
    { return left <= right; }
    native_bool gte() const final
    { return left >= right; }
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfloat-equal"
    native_bool equal() const final
    { return left == right; }
#pragma clang diagnostic pop
    native_bool is_positive() const final
    { return left > 0; }
    native_bool is_negative() const final
    { return left < 0; }
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfloat-equal"
    native_bool is_zero() const final
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
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch-enum"
    switch(n->type)
    {
      case object_type::integer:
      {
        i_ops.left = expect_object<obj::integer>(n)->data;
        return i_ops;
      }
      case object_type::real:
      {
        r_ops.left = expect_object<obj::real>(n)->data;
        return r_ops;
      }
      default:
      /* TODO: Exception type. */
      { throw std::runtime_error{ fmt::format("(left_ops) not a number: {}", detail::to_string(n)) }; }
    }
#pragma clang diagnostic pop
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
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch-enum"
    switch(n->type)
    {
      case object_type::integer:
      {
        i_ops.right = expect_object<obj::integer>(n)->data;
        return i_ops;
      }
      case object_type::real:
      {
        r_ops.right = expect_object<obj::real>(n)->data;
        return r_ops;
      }
      default:
      /* TODO: Exception type. */
      { throw std::runtime_error{ fmt::format("(right_ops) not a number: {}", detail::to_string(n)) }; }
    }
#pragma clang diagnostic pop
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
  object_ptr add(obj::integer_ptr const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).add(); }
  object_ptr add(object_ptr const l, obj::integer_ptr const r)
  { return with(left_ops(l), right_ops(r)).add(); }
  native_integer add(obj::integer_ptr const l, obj::integer_ptr const r)
  { return l->data + r->data; }
  native_real add(obj::real_ptr const l, obj::real_ptr const r)
  { return l->data + r->data; }
  native_real add(obj::real_ptr const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).add_real(); }
  native_real add(object_ptr const l, obj::real_ptr const r)
  { return with(left_ops(l), right_ops(r)).add_real(); }
  native_real add(obj::real_ptr const l, obj::integer_ptr const r)
  { return l->data + r->data; }
  native_real add(obj::integer_ptr const l, obj::real_ptr const r)
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
  object_ptr sub(obj::integer_ptr const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).subtract(); }
  object_ptr sub(object_ptr const l, obj::integer_ptr const r)
  { return with(left_ops(l), right_ops(r)).subtract(); }
  native_integer sub(obj::integer_ptr const l, obj::integer_ptr const r)
  { return l->data - r->data; }
  native_real sub(obj::real_ptr const l, obj::real_ptr const r)
  { return l->data - r->data; }
  native_real sub(obj::real_ptr const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).sub_real(); }
  native_real sub(object_ptr const l, obj::real_ptr const r)
  { return with(left_ops(l), right_ops(r)).sub_real(); }
  native_real sub(obj::real_ptr const l, obj::integer_ptr const r)
  { return l->data - r->data; }
  native_real sub(obj::integer_ptr const l, obj::real_ptr const r)
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
  object_ptr div(obj::integer_ptr const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).divide(); }
  object_ptr div(object_ptr const l, obj::integer_ptr const r)
  { return with(left_ops(l), right_ops(r)).divide(); }
  native_integer div(obj::integer_ptr const l, obj::integer_ptr const r)
  { return l->data / r->data; }
  native_real div(obj::real_ptr const l, obj::real_ptr const r)
  { return l->data / r->data; }
  native_real div(obj::real_ptr const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).div_real(); }
  native_real div(object_ptr const l, obj::real_ptr const r)
  { return with(left_ops(l), right_ops(r)).div_real(); }
  native_real div(obj::real_ptr const l, obj::integer_ptr const r)
  { return l->data / r->data; }
  native_real div(obj::integer_ptr const l, obj::real_ptr const r)
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
  object_ptr mul(obj::integer_ptr const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).multiply(); }
  object_ptr mul(object_ptr const l, obj::integer_ptr const r)
  { return with(left_ops(l), right_ops(r)).multiply(); }
  native_integer mul(obj::integer_ptr const l, obj::integer_ptr const r)
  { return l->data * r->data; }
  native_real mul(obj::real_ptr const l, obj::real_ptr const r)
  { return l->data * r->data; }
  native_real mul(obj::real_ptr const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).mul_real(); }
  native_real mul(object_ptr const l, obj::real_ptr const r)
  { return with(left_ops(l), right_ops(r)).mul_real(); }
  native_real mul(obj::real_ptr const l, obj::integer_ptr const r)
  { return l->data * r->data; }
  native_real mul(obj::integer_ptr const l, obj::real_ptr const r)
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
  bool lt(obj::integer_ptr const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).lt(); }
  bool lt(object_ptr const l, obj::integer_ptr const r)
  { return with(left_ops(l), right_ops(r)).lt(); }
  bool lt(obj::integer_ptr const l, obj::integer_ptr const r)
  { return l->data < r->data; }
  bool lt(obj::real_ptr const l, obj::real_ptr const r)
  { return l->data < r->data; }
  bool lt(obj::real_ptr const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).lt(); }
  bool lt(object_ptr const l, obj::real_ptr const r)
  { return with(left_ops(l), right_ops(r)).lt(); }
  bool lt(obj::real_ptr const l, obj::integer_ptr const r)
  { return with(left_ops(l), right_ops(r)).lt(); }
  bool lt(obj::integer_ptr const l, obj::real_ptr const r)
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
  bool lte(obj::integer_ptr const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).lte(); }
  bool lte(object_ptr const l, obj::integer_ptr const r)
  { return with(left_ops(l), right_ops(r)).lte(); }
  bool lte(obj::integer_ptr const l, obj::integer_ptr const r)
  { return l->data <= r->data; }
  bool lte(obj::real_ptr const l, obj::real_ptr const r)
  { return l->data <= r->data; }
  bool lte(obj::real_ptr const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).lte(); }
  bool lte(object_ptr const l, obj::real_ptr const r)
  { return with(left_ops(l), right_ops(r)).lte(); }
  bool lte(obj::real_ptr const l, obj::integer_ptr const r)
  { return with(left_ops(l), right_ops(r)).lte(); }
  bool lte(obj::integer_ptr const l, obj::real_ptr const r)
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
  object_ptr min(obj::integer_ptr const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).min(); }
  object_ptr min(object_ptr const l, obj::integer_ptr const r)
  { return with(left_ops(l), right_ops(r)).min(); }
  native_integer min(obj::integer_ptr const l, obj::integer_ptr const r)
  { return std::min(l->data, r->data); }
  native_real min(obj::real_ptr const l, obj::real_ptr const r)
  { return std::min(l->data, r->data); }
  native_real min(obj::real_ptr const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).min_real(); }
  native_real min(object_ptr const l, obj::real_ptr const r)
  { return with(left_ops(l), right_ops(r)).min_real(); }
  native_real min(obj::real_ptr const l, obj::integer_ptr const r)
  { return std::min(l->data, static_cast<native_real>(r->data)); }
  native_real min(obj::integer_ptr const l, obj::real_ptr const r)
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
  object_ptr max(obj::integer_ptr const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).max(); }
  object_ptr max(object_ptr const l, obj::integer_ptr const r)
  { return with(left_ops(l), right_ops(r)).max(); }
  native_integer max(obj::integer_ptr const l, obj::integer_ptr const r)
  { return std::max(l->data, r->data); }
  native_real max(obj::real_ptr const l, obj::real_ptr const r)
  { return std::max(l->data, r->data); }
  native_real max(obj::real_ptr const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).max_real(); }
  native_real max(object_ptr const l, obj::real_ptr const r)
  { return with(left_ops(l), right_ops(r)).max_real(); }
  native_real max(obj::real_ptr const l, obj::integer_ptr const r)
  { return std::max(l->data, static_cast<native_real>(r->data)); }
  native_real max(obj::integer_ptr const l, obj::real_ptr const r)
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
  native_integer abs(obj::integer_ptr const l)
  { return std::abs(l->data); }
  native_real abs(obj::real_ptr const l)
  { return std::fabs(l->data); }
  native_integer abs(native_integer const l)
  { return std::abs(l); }
  native_real abs(native_real const l)
  { return std::fabs(l); }

  native_real sqrt(object_ptr const l)
  {
    return visit_object
    (
      l,
      [](auto const typed_l) -> native_real
      {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<T>)
        { return std::sqrt(typed_l->to_real()); }
        else
        { throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) }; }
      }
    );
  }
  native_real sqrt(obj::integer_ptr const l)
  { return std::sqrt(l->data); }
  native_real sqrt(obj::real_ptr const l)
  { return std::sqrt(l->data); }
  native_real sqrt(native_integer const l)
  { return std::sqrt(l); }
  native_real sqrt(native_real const l)
  { return std::sqrt(l); }

  native_real pow(object_ptr const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).pow(); }
  native_real pow(obj::integer_ptr const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).pow(); }
  native_real pow(object_ptr const l, obj::integer_ptr const r)
  { return with(left_ops(l), right_ops(r)).pow(); }
  native_real pow(obj::integer_ptr const l, obj::integer_ptr const r)
  { return std::pow(l->data, r->data); }
  native_real pow(obj::real_ptr const l, obj::real_ptr const r)
  { return std::pow(l->data, r->data); }
  native_real pow(obj::real_ptr const l, object_ptr const r)
  { return with(left_ops(l), right_ops(r)).pow(); }
  native_real pow(object_ptr const l, obj::real_ptr const r)
  { return with(left_ops(l), right_ops(r)).pow(); }
  native_real pow(obj::real_ptr const l, obj::integer_ptr const r)
  { return std::pow(l->data, r->data); }
  native_real pow(obj::integer_ptr const l, obj::real_ptr const r)
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
    return visit_object
    (
      l,
      [](auto const typed_l) -> native_integer
      {
        using T = typename decltype(typed_l)::value_type;

        if constexpr(behavior::numberable<T>)
        { return typed_l->to_integer(); }
        else
        { throw std::runtime_error{ fmt::format("not a number: {}", typed_l->to_string()) }; }
      }
    );
  }
  native_integer to_int(obj::integer_ptr const l)
  { return l->data; }
  native_integer to_int(obj::real_ptr const l)
  { return static_cast<native_integer>(l->data); }
  native_integer to_int(native_integer const l)
  { return l; }
  native_integer to_int(native_real const l)
  { return static_cast<native_integer>(l); }
}
