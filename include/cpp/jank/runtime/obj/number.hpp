#pragma once

namespace jank::runtime::obj
{
  struct number : virtual gc
  {
    virtual ~number() = default;

    /* TODO: Should this be called to_integer/to_number to be consistent with to_string? */
    virtual native_integer get_integer() const = 0;
    virtual native_real get_real() const = 0;
  };

  struct boolean : object
  {
    boolean() = default;
    virtual ~boolean() = default;
    boolean(boolean &&) noexcept = default;
    boolean(boolean const &) = default;
    boolean(native_bool const d);

    native_bool equal(object const &) const override;
    native_string to_string() const override;
    void to_string(fmt::memory_buffer &buff) const override;
    native_integer to_hash() const override;

    boolean const* as_boolean() const override;

    native_bool data{};
  };

  struct integer : object, number
  {
    integer() = default;
    integer(integer &&) noexcept = default;
    integer(integer const &) = default;
    integer(native_integer const d);

    static native_box<integer> create(native_integer const &n);

    native_bool equal(object const &) const override;
    native_string to_string() const override;
    void to_string(fmt::memory_buffer &) const override;
    native_integer to_hash() const override;

    native_integer get_integer() const override;
    native_real get_real() const override;

    integer const* as_integer() const override;
    number const* as_number() const override;

    native_integer data{};
  };

  struct real : object, number
  {
    real() = default;
    real(real &&) noexcept = default;
    real(real const &) = default;
    real(native_real const d);

    native_bool equal(object const &) const override;
    native_string to_string() const override;
    void to_string(fmt::memory_buffer &buff) const override;
    native_integer to_hash() const override;

    native_integer get_integer() const override;
    native_real get_real() const override;

    real const* as_real() const override;
    number const* as_number() const override;

    native_real data{};
  };

  /* TODO: Find a better namespace for these. */
  struct integer_ops;
  struct real_ops;
  struct number_ops
  {
    virtual ~number_ops() = default;

    virtual number_ops const& combine(number_ops const&) const = 0;
    virtual number_ops const& with(integer_ops const&) const = 0;
    virtual number_ops const& with(real_ops const&) const = 0;

    /* TODO: Return number_ptr? */
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
  object_ptr _gen_plus_(object_ptr l, object_ptr r);
  object_ptr _gen_minus_(object_ptr l, object_ptr r);
  object_ptr _gen_asterisk_(object_ptr l, object_ptr r);
  object_ptr div(object_ptr l, object_ptr r);
  object_ptr mod(object_ptr l, object_ptr r);
  object_ptr _gen_less_(object_ptr l, object_ptr r);
  object_ptr _gen_less__gen_equal_(object_ptr l, object_ptr r);
  object_ptr min(object_ptr l, object_ptr r);
  object_ptr max(object_ptr l, object_ptr r);
}
