#include <iostream>
#include <random>
#include <cmath>

#include <jank/runtime/obj/number.hpp>

namespace jank::runtime::obj
{
  /***** boolean *****/
  detail::boolean_type boolean::equal(object const &o) const
  {
    auto const *b(o.as_boolean());
    if(!b)
    { return false; }

    return data == b->data;
  }
  detail::string_type boolean::to_string() const
  /* TODO: Optimize. */
  { return data ? "true" : "false"; }
  detail::integer_type boolean::to_hash() const
  { return data ? 1 : 0; }
  boolean const* boolean::as_boolean() const
  { return this; }

  /***** integer *****/
  runtime::detail::box_type<integer> integer::create(runtime::detail::integer_type const &n)
  { return make_box<integer>(n); }
  detail::boolean_type integer::equal(object const &o) const
  {
    auto const *i(o.as_integer());
    if(!i)
    { return false; }

    return data == i->data;
  }
  detail::string_type integer::to_string() const
  /* TODO: Optimize by rendering into string_type. */
  { return std::to_string(data); }
  detail::integer_type integer::to_hash() const
  { return data; }
  detail::integer_type integer::get_integer() const
  { return data; }
  detail::real_type integer::get_real() const
  { return data; }
  integer const* integer::as_integer() const
  { return this; }
  number const* integer::as_number() const
  { return this; }

  /***** real *****/
  detail::boolean_type real::equal(object const &o) const
  {
    auto const *r(o.as_real());
    if(!r)
    { return false; }

    std::hash<detail::real_type> hasher{};
    return hasher(data) == hasher(r->data);
  }
  detail::string_type real::to_string() const
  /* TODO: Optimize by rendering into string_type. */
  { return std::to_string(data); }
  detail::integer_type real::to_hash() const
  { return static_cast<detail::integer_type>(data); }
  detail::integer_type real::get_integer() const
  { return data; }
  detail::real_type real::get_real() const
  { return data; }
  real const* real::as_real() const
  { return this; }
  number const* real::as_number() const
  { return this; }

  struct integer_ops;
  struct real_ops;
  struct number_ops
  {
    virtual ~number_ops() = default;

    virtual number_ops const& combine(number_ops const&) const = 0;
    virtual number_ops const& with(integer_ops const&) const = 0;
    virtual number_ops const& with(real_ops const&) const = 0;

    /* TODO: Return number_ptr */
    virtual object_ptr add() const = 0;
    virtual object_ptr subtract() const = 0;
    virtual object_ptr multiply() const = 0;
    virtual object_ptr divide() const = 0;
    virtual object_ptr remainder() const = 0;
    virtual object_ptr inc() const = 0;
    virtual object_ptr dec() const = 0;
    virtual object_ptr negate() const = 0;
    virtual object_ptr abs() const = 0;
    virtual object_ptr min() const = 0;
    virtual object_ptr max() const = 0;
    virtual detail::boolean_type lt() const = 0;
    virtual detail::boolean_type lte() const = 0;
    virtual detail::boolean_type gte() const = 0;
    virtual detail::boolean_type equal() const = 0;
  };

  struct integer_ops : number_ops
  {
    number_ops const& combine(number_ops const &l) const override
    { return l.with(*this); }
    number_ops const& with(integer_ops const&) const override;
    number_ops const& with(real_ops const&) const override;
    object_ptr add() const override
    { return make_box<integer>(left + right); }
    object_ptr subtract() const override
    { return make_box<integer>(left - right); }
    object_ptr multiply() const override
    { return make_box<integer>(left * right); }
    object_ptr divide() const override
    { return make_box<integer>(left / right); }
    object_ptr remainder() const override
    { return make_box<integer>(left % right); }
    object_ptr inc() const override
    { return make_box<integer>(left + 1); }
    object_ptr dec() const override
    { return make_box<integer>(left - 1); }
    object_ptr negate() const override
    { return make_box<integer>(-left); }
    object_ptr abs() const override
    { return make_box<integer>(std::labs(left)); }
    object_ptr min() const override
    { return make_box<integer>(std::min(left, right)); }
    object_ptr max() const override
    { return make_box<integer>(std::max(left, right)); }
    detail::boolean_type lt() const override
    { return left < right; }
    detail::boolean_type lte() const override
    { return left <= right; }
    detail::boolean_type gte() const override
    { return left >= right; }
    detail::boolean_type equal() const override
    { return left == right; }

    detail::integer_type left, right;
  };

  struct real_ops : number_ops
  {
    number_ops const& combine(number_ops const &l) const override
    { return l.with(*this); }
    number_ops const& with(integer_ops const&) const override;
    number_ops const& with(real_ops const&) const override;
    object_ptr add() const override
    { return make_box<real>(left + right); }
    object_ptr subtract() const override
    { return make_box<real>(left - right); }
    object_ptr multiply() const override
    { return make_box<real>(left * right); }
    object_ptr divide() const override
    { return make_box<real>(left / right); }
    object_ptr remainder() const override
    { return make_box<real>(std::fmod(left, right)); }
    object_ptr inc() const override
    { return make_box<real>(left + 1); }
    object_ptr dec() const override
    { return make_box<real>(right + 1); }
    object_ptr negate() const override
    { return make_box<real>(-left); }
    object_ptr abs() const override
    { return make_box<real>(std::fabs(left)); }
    object_ptr min() const override
    { return make_box<real>(std::min(left, right)); }
    object_ptr max() const override
    { return make_box<real>(std::max(left, right)); }
    detail::boolean_type lt() const override
    { return left < right; }
    detail::boolean_type lte() const override
    { return left <= right; }
    detail::boolean_type gte() const override
    { return left >= right; }
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfloat-equal"
    detail::boolean_type equal() const override
    { return left == right; }
#pragma clang diagnostic pop

    detail::real_type left, right;
  };

  static thread_local integer_ops i_ops;
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

  number_ops& left_ops(object_ptr const &n)
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
    throw detail::string_type{ "(left_ops) not a number: " } + n->to_string();
  }

  number_ops& right_ops(object_ptr const &n)
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
    throw detail::string_type{ "(right_ops) not a number: " } + n->to_string();
  }

  object_ptr rand()
  {
    static std::uniform_real_distribution<detail::real_type> distribution(0.0, 1.0);
    static std::mt19937 generator;
    return make_box<real>(distribution(generator));
  }

  /* + */
  object_ptr _gen_plus_(object_ptr const &l, object_ptr const &r)
  { return right_ops(r).combine(left_ops(l)).add(); }

  /* - */
  object_ptr _gen_minus_(object_ptr const &l, object_ptr const &r)
  { return right_ops(r).combine(left_ops(l)).subtract(); }

  /* * */
  object_ptr _gen_asterisk_(object_ptr const &l, object_ptr const &r)
  { return right_ops(r).combine(left_ops(l)).multiply(); }

  /* TODO: Rename to / once the parser supports it. */
  /* / */
  object_ptr div(object_ptr const &l, object_ptr const &r)
  { return right_ops(r).combine(left_ops(l)).divide(); }

  object_ptr mod(object_ptr const &l, object_ptr const &r)
  { return right_ops(r).combine(left_ops(l)).remainder(); }

  /* < */
  object_ptr _gen_less_(object_ptr const &l, object_ptr const &r)
  {
    return make_box<boolean>
    (right_ops(r).combine(left_ops(l)).lt());
  }

  /* <= */
  object_ptr _gen_less__gen_equal_(object_ptr const &l, object_ptr const &r)
  {
    return make_box<boolean>
    (right_ops(r).combine(left_ops(l)).lte());
  }

  /* ->int */
  object_ptr _gen_minus__gen_greater_int(object_ptr const &o)
  {
    auto const * const n(o->as_number());
    if(!n)
    {
      /* TODO: Throw error. */
      std::cout << "(->int) not a number: " << *o << std::endl;
      return JANK_NIL;
    }
    return make_box<integer>(n->get_integer());
  }

  /* ->float */
  object_ptr _gen_minus__gen_greater_float(object_ptr const &o)
  {
    auto const * const n(o->as_number());
    if(!n)
    {
      /* TODO: Throw error. */
      std::cout << "(->float) not a number: " << *o << std::endl;
      return JANK_NIL;
    }
    return make_box<real>(n->get_real());
  }

  object_ptr inc(object_ptr const &n)
  { return left_ops(n).inc(); }

  object_ptr dec(object_ptr const &n)
  { return left_ops(n).dec(); }

  object_ptr sqrt(object_ptr const &o)
  {
    auto const * const n(o->as_number());
    if(!n)
    {
      /* TODO: Throw error. */
      std::cout << "(sqrt) not a number: " << *o << std::endl;
      return JANK_NIL;
    }
    return make_box<real>(std::sqrt(n->get_real()));
  }

  object_ptr tan(object_ptr const &o)
  {
    auto const * const n(o->as_number());
    if(!n)
    {
      /* TODO: Throw error. */
      std::cout << "(tan) not a number: " << *o << std::endl;
      return JANK_NIL;
    }
    return make_box<real>(std::tan(n->get_real()));
  }

  object_ptr pow(object_ptr const &l, object_ptr const &r)
  {
    auto const * const l_num(l->as_number());
    auto const * const r_num(r->as_number());
    if(!l_num || !r_num)
    {
      /* TODO: Throw error. */
      std::cout << "(pow) not a number: " << *l << " and " << *r << std::endl;
      return JANK_NIL;
    }
    return make_box<real>(std::pow(l_num->get_real(), r_num->get_real()));
  }

  object_ptr abs(object_ptr const &n)
  { return left_ops(n).abs(); }

  object_ptr min(object_ptr const &l, object_ptr const &r)
  { return right_ops(r).combine(left_ops(l)).min(); }

  object_ptr max(object_ptr const &l, object_ptr const &r)
  { return right_ops(r).combine(left_ops(l)).max(); }
}
