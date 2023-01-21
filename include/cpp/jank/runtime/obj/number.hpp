#pragma once

namespace jank::runtime::obj
{
  struct number : virtual gc
  {
    virtual ~number() = default;

    virtual native_integer get_integer() const = 0;
    virtual native_real get_real() const = 0;
  };

  struct boolean : object
  {
    boolean() = default;
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

  object_ptr rand();
  object_ptr _gen_plus_(object_ptr l, object_ptr r);
  object_ptr _gen_minus_(object_ptr l, object_ptr r);
  object_ptr _gen_asterisk_(object_ptr l, object_ptr r);
  object_ptr div(object_ptr l, object_ptr r);
  object_ptr mod(object_ptr l, object_ptr r);
  object_ptr _gen_less_(object_ptr l, object_ptr r);
  object_ptr _gen_less__gen_equal_(object_ptr l, object_ptr r);
  object_ptr _gen_minus__gen_greater_int(object_ptr o);
  object_ptr _gen_minus__gen_greater_float(object_ptr o);
  object_ptr inc(object_ptr n);
  object_ptr dec(object_ptr n);
  object_ptr sqrt(object_ptr o);
  object_ptr tan(object_ptr o);
  object_ptr pow(object_ptr l, object_ptr r);
  object_ptr abs(object_ptr n);
  object_ptr min(object_ptr l, object_ptr r);
  object_ptr max(object_ptr l, object_ptr r);
}
