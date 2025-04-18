#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using boolean_ref = oref<struct boolean>;

  struct boolean : gc
  {
    static constexpr object_type obj_type{ object_type::boolean };
    static constexpr native_bool pointer_free{ true };

    boolean() = default;
    boolean(boolean &&) noexcept = default;
    boolean(boolean const &) = default;
    boolean(native_bool const d);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    void to_string(util::string_builder &buff) const;
    jtl::immutable_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::comparable */
    native_integer compare(object const &) const;

    /* behavior::comparable extended */
    native_integer compare(boolean const &) const;

    object base{ obj_type };
    native_bool data{};
  };

  using integer_ref = oref<struct integer>;

  struct integer : gc
  {
    static constexpr object_type obj_type{ object_type::integer };
    static constexpr native_bool pointer_free{ true };

    integer() = default;
    integer(integer &&) noexcept = default;
    integer(integer const &) = default;
    integer(native_integer const d);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    void to_string(util::string_builder &buff) const;
    jtl::immutable_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::comparable */
    native_integer compare(object const &) const;

    /* behavior::comparable extended */
    native_integer compare(integer const &) const;

    /* behavior::number_like */
    native_integer to_integer() const;
    native_real to_real() const;

    native_integer data{};
    object base{ obj_type };
  };

  using real_ref = oref<struct real>;

  struct real : gc
  {
    static constexpr object_type obj_type{ object_type::real };
    static constexpr native_bool pointer_free{ true };

    real() = default;
    real(real &&) noexcept = default;
    real(real const &) = default;
    real(native_real const d);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    void to_string(util::string_builder &buff) const;
    jtl::immutable_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::comparable */
    native_integer compare(object const &) const;

    /* behavior::comparable extended */
    native_integer compare(real const &) const;

    /* behavior::number_like */
    native_integer to_integer() const;
    native_real to_real() const;

    native_real data{};
    object base{ obj_type };
  };
}

namespace jank::runtime
{
  /* NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables) */
  extern obj::boolean_ref jank_true;
  /* NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables) */
  extern obj::boolean_ref jank_false;
}
