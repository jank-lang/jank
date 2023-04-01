#pragma once

namespace jank::runtime::obj
{
  struct number
  {
    virtual ~number() = default;

    /* TODO: Should this be called to_integer/to_number to be consistent with to_string? */
    virtual native_integer get_integer() const = 0;
    virtual native_real get_real() const = 0;
  };
  using number_ptr = native_box<number>;

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
  using boolean_ptr = native_box<boolean>;

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
  using integer_ptr = native_box<integer>;

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
  using real_ptr = native_box<real>;

  /* TODO: Find a better namespace for these. */
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
  object_ptr add(integer_ptr l, object_ptr r);
  object_ptr add(object_ptr l, integer_ptr r);
  native_integer add(integer_ptr l, integer_ptr r);
  native_real add(real_ptr l, real_ptr r);
  native_real add(real_ptr l, object_ptr r);
  native_real add(object_ptr l, real_ptr r);
  native_real add(real_ptr l, integer_ptr r);
  native_real add(integer_ptr l, real_ptr r);

  native_real add(object_ptr l, native_real r);
  native_real add(native_real l, object_ptr r);
  native_real add(native_real l, native_real r);

  native_real add(native_integer l, native_real r);
  native_real add(native_real l, native_integer r);

  object_ptr add(object_ptr l, native_integer r);
  object_ptr add(native_integer l, object_ptr r);
  native_integer add(native_integer l, native_integer r);

  object_ptr sub(object_ptr l, object_ptr r);
  object_ptr sub(integer_ptr l, object_ptr r);
  object_ptr sub(object_ptr l, integer_ptr r);
  native_integer sub(integer_ptr l, integer_ptr r);
  native_real sub(real_ptr l, real_ptr r);
  native_real sub(real_ptr l, object_ptr r);
  native_real sub(object_ptr l, real_ptr r);
  native_real sub(real_ptr l, integer_ptr r);
  native_real sub(integer_ptr l, real_ptr r);

  native_real sub(object_ptr l, native_real r);
  native_real sub(native_real l, object_ptr r);
  native_real sub(native_real l, native_real r);

  native_real sub(native_integer l, native_real r);
  native_real sub(native_real l, native_integer r);

  object_ptr sub(object_ptr l, native_integer r);
  object_ptr sub(native_integer l, object_ptr r);
  native_integer sub(native_integer l, native_integer r);

  object_ptr div(object_ptr l, object_ptr r);
  object_ptr div(integer_ptr l, object_ptr r);
  object_ptr div(object_ptr l, integer_ptr r);
  native_integer div(integer_ptr l, integer_ptr r);
  native_real div(real_ptr l, real_ptr r);
  native_real div(real_ptr l, object_ptr r);
  native_real div(object_ptr l, real_ptr r);
  native_real div(real_ptr l, integer_ptr r);
  native_real div(integer_ptr l, real_ptr r);

  native_real div(object_ptr l, native_real r);
  native_real div(native_real l, object_ptr r);
  native_real div(native_real l, native_real r);

  native_real div(native_integer l, native_real r);
  native_real div(native_real l, native_integer r);

  object_ptr div(object_ptr l, native_integer r);
  object_ptr div(native_integer l, object_ptr r);
  native_integer div(native_integer l, native_integer r);

  object_ptr mul(object_ptr l, object_ptr r);
  object_ptr mul(integer_ptr l, object_ptr r);
  object_ptr mul(object_ptr l, integer_ptr r);
  native_integer mul(integer_ptr l, integer_ptr r);
  native_real mul(real_ptr l, real_ptr r);
  native_real mul(real_ptr l, object_ptr r);
  native_real mul(object_ptr l, real_ptr r);
  native_real mul(real_ptr l, integer_ptr r);
  native_real mul(integer_ptr l, real_ptr r);

  native_real mul(object_ptr l, native_real r);
  native_real mul(native_real l, object_ptr r);
  native_real mul(native_real l, native_real r);

  native_real mul(native_integer l, native_real r);
  native_real mul(native_real l, native_integer r);

  object_ptr mul(object_ptr l, native_integer r);
  object_ptr mul(native_integer l, object_ptr r);
  native_integer mul(native_integer l, native_integer r);

  bool lt(object_ptr l, object_ptr r);
  bool lt(integer_ptr l, object_ptr r);
  bool lt(object_ptr l, integer_ptr r);
  bool lt(integer_ptr const l, integer_ptr const r);
  bool lt(real_ptr const l, real_ptr const r);
  bool lt(real_ptr l, object_ptr r);
  bool lt(object_ptr l, real_ptr r);
  bool lt(real_ptr l, integer_ptr r);
  bool lt(integer_ptr l, real_ptr r);

  bool lt(object_ptr l, native_real r);
  bool lt(native_real l, object_ptr r);
  bool lt(native_real l, native_real r);

  bool lt(native_integer l, native_real r);
  bool lt(native_real l, native_integer r);

  bool lt(object_ptr l, native_integer r);
  bool lt(native_integer l, object_ptr r);
  bool lt(native_integer l, native_integer r);

  bool lte(object_ptr l, object_ptr r);
  bool lte(integer_ptr l, object_ptr r);
  bool lte(object_ptr l, integer_ptr r);
  bool lte(integer_ptr const l, integer_ptr const r);
  bool lte(real_ptr const l, real_ptr const r);
  bool lte(real_ptr l, object_ptr r);
  bool lte(object_ptr l, real_ptr r);
  bool lte(real_ptr l, integer_ptr r);
  bool lte(integer_ptr l, real_ptr r);

  bool lte(object_ptr l, native_real r);
  bool lte(native_real l, object_ptr r);
  bool lte(native_real l, native_real r);

  bool lte(native_integer l, native_real r);
  bool lte(native_real l, native_integer r);

  bool lte(object_ptr l, native_integer r);
  bool lte(native_integer l, object_ptr r);
  bool lte(native_integer l, native_integer r);

  object_ptr min(object_ptr l, object_ptr r);
  object_ptr min(integer_ptr l, object_ptr r);
  object_ptr min(object_ptr l, integer_ptr r);
  native_integer min(integer_ptr l, integer_ptr r);
  native_real min(real_ptr l, real_ptr r);
  native_real min(real_ptr l, object_ptr r);
  native_real min(object_ptr l, real_ptr r);
  native_real min(real_ptr l, integer_ptr r);
  native_real min(integer_ptr l, real_ptr r);

  native_real min(object_ptr l, native_real r);
  native_real min(native_real l, object_ptr r);
  native_real min(native_real l, native_real r);

  native_real min(native_integer l, native_real r);
  native_real min(native_real l, native_integer r);

  object_ptr min(object_ptr l, native_integer r);
  object_ptr min(native_integer l, object_ptr r);
  native_integer min(native_integer l, native_integer r);

  object_ptr max(object_ptr l, object_ptr r);
  object_ptr max(integer_ptr l, object_ptr r);
  object_ptr max(object_ptr l, integer_ptr r);
  native_integer max(integer_ptr l, integer_ptr r);
  native_real max(real_ptr l, real_ptr r);
  native_real max(real_ptr l, object_ptr r);
  native_real max(object_ptr l, real_ptr r);
  native_real max(real_ptr l, integer_ptr r);
  native_real max(integer_ptr l, real_ptr r);

  native_real max(object_ptr l, native_real r);
  native_real max(native_real l, object_ptr r);
  native_real max(native_real l, native_real r);

  native_real max(native_integer l, native_real r);
  native_real max(native_real l, native_integer r);

  object_ptr max(object_ptr l, native_integer r);
  object_ptr max(native_integer l, object_ptr r);
  native_integer max(native_integer l, native_integer r);

  object_ptr abs(object_ptr l);
  native_integer abs(integer_ptr l);
  native_real abs(real_ptr l);
  native_integer abs(native_integer l);
  native_real abs(native_real l);

  native_real sqrt(object_ptr l);
  native_real sqrt(integer_ptr l);
  native_real sqrt(real_ptr l);
  native_real sqrt(native_integer l);
  native_real sqrt(native_real l);

  native_real pow(object_ptr l, object_ptr r);
  native_real pow(integer_ptr l, object_ptr r);
  native_real pow(object_ptr l, integer_ptr r);
  native_real pow(integer_ptr l, integer_ptr r);
  native_real pow(real_ptr l, real_ptr r);
  native_real pow(real_ptr l, object_ptr r);
  native_real pow(object_ptr l, real_ptr r);
  native_real pow(real_ptr l, integer_ptr r);
  native_real pow(integer_ptr l, real_ptr r);

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
  native_integer to_int(integer_ptr l);
  native_integer to_int(real_ptr l);
  native_integer to_int(native_integer l);
  native_integer to_int(native_real l);
}
