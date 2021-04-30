#pragma once

#include <prelude/object.hpp>

namespace jank
{
  struct number
  {
    virtual detail::integer_type get_integer() const = 0;
    virtual detail::real_type get_real() const = 0;
  };

  struct boolean : object
  {
    boolean() = default;
    boolean(boolean &&) = default;
    boolean(boolean const &) = default;
    boolean(detail::boolean_type const d)
      : data{ d }
    { }

    detail::boolean_type equal(object const &) const override;
    detail::string_type to_string() const override;
    detail::integer_type to_hash() const override;

    boolean const* as_boolean() const override;

    detail::boolean_type data{};
  };
  extern object_ptr JANK_TRUE;
  extern object_ptr JANK_FALSE;

  struct integer : object, number
  {
    integer() = default;
    integer(integer &&) = default;
    integer(integer const &) = default;
    integer(detail::integer_type const d)
      : data{ d }
    { }

    detail::boolean_type equal(object const &) const override;
    detail::string_type to_string() const override;
    detail::integer_type to_hash() const override;

    detail::integer_type get_integer() const override;
    detail::real_type get_real() const override;

    integer const* as_integer() const override;
    number const* as_number() const override;

    detail::integer_type data{};
  };

  struct real : object, number
  {
    real() = default;
    real(real &&) = default;
    real(real const &) = default;
    real(detail::real_type const d)
      : data{ d }
    { }

    detail::boolean_type equal(object const &) const override;
    detail::string_type to_string() const override;
    detail::integer_type to_hash() const override;

    detail::integer_type get_integer() const override;
    detail::real_type get_real() const override;

    real const* as_real() const override;
    number const* as_number() const override;

    detail::real_type data{};
  };

  object_ptr rand();
  object_ptr _gen_plus_(object_ptr const &l, object_ptr const &r);
  object_ptr _gen_minus_(object_ptr const &l, object_ptr const &r);
  object_ptr _gen_asterisk_(object_ptr const &l, object_ptr const &r);
  object_ptr div(object_ptr const &l, object_ptr const &r);
  object_ptr mod(object_ptr const &l, object_ptr const &r);
  object_ptr _gen_less_(object_ptr const &l, object_ptr const &r);
  object_ptr _gen_less__gen_equal_(object_ptr const &l, object_ptr const &r);
  object_ptr _gen_minus__gen_greater_int(object_ptr const &o);
  object_ptr _gen_minus__gen_greater_float(object_ptr const &o);
  object_ptr inc(object_ptr const &n);
  object_ptr dec(object_ptr const &n);
  object_ptr sqrt(object_ptr const &o);
  object_ptr tan(object_ptr const &o);
  object_ptr pow(object_ptr const &l, object_ptr const &r);
  object_ptr abs(object_ptr const &n);
  object_ptr min(object_ptr const &l, object_ptr const &r);
  object_ptr max(object_ptr const &l, object_ptr const &r);
}
