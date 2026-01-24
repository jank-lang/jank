#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using boolean_ref = oref<struct boolean>;

  struct boolean : object
  {
    static constexpr object_type obj_type{ object_type::boolean };
    static constexpr bool pointer_free{ true };

    boolean();
    boolean(boolean &&) noexcept = default;
    boolean(boolean const &) = default;
    boolean(bool const d);

    /* behavior::object_like */
    bool equal(object const &) const override;
    jtl::immutable_string to_string() const override;
    void to_string(jtl::string_builder &buff) const override;
    jtl::immutable_string to_code_string() const override;
    uhash to_hash() const override;

    /* behavior::comparable */
    i64 compare(object const &) const;

    /* behavior::comparable extended */
    i64 compare(boolean const &) const;

    /* behavior::number_like */
    i64 to_integer() const;
    f64 to_real() const;

    /*** XXX: Everything here is immutable after initialization. ***/
    bool data{};
  };

  using integer_ref = oref<struct integer>;

  struct integer : object
  {
    static constexpr object_type obj_type{ object_type::integer };
    static constexpr bool pointer_free{ true };

    integer();
    integer(integer &&) noexcept = default;
    integer(integer const &) = default;
    integer(i64 const d);

    /* behavior::object_like */
    bool equal(object const &) const override;
    jtl::immutable_string to_string() const override;
    void to_string(jtl::string_builder &buff) const override;
    jtl::immutable_string to_code_string() const override;
    uhash to_hash() const override;

    /* behavior::comparable */
    i64 compare(object const &) const;

    /* behavior::comparable extended */
    i64 compare(integer const &) const;

    /* behavior::number_like */
    i64 to_integer() const;
    f64 to_real() const;

    /*** XXX: Everything here is immutable after initialization. ***/
    /* TODO: Is it faster to have the data first or the base first? */
    i64 data{};
  };

  using real_ref = oref<struct real>;

  struct real : object
  {
    static constexpr object_type obj_type{ object_type::real };
    static constexpr bool pointer_free{ true };

    real();
    real(real &&) noexcept = default;
    real(real const &) = default;
    real(f64 const d);

    /* behavior::object_like */
    bool equal(object const &) const override;
    jtl::immutable_string to_string() const override;
    void to_string(jtl::string_builder &buff) const override;
    jtl::immutable_string to_code_string() const override;
    uhash to_hash() const override;

    /* behavior::comparable */
    i64 compare(object const &) const;

    /* behavior::comparable extended */
    i64 compare(real const &) const;

    /* behavior::number_like */
    i64 to_integer() const;
    f64 to_real() const;

    /*** XXX: Everything here is immutable after initialization. ***/
    object base{ obj_type };
    f64 data{};
  };
}

namespace jank::runtime
{
  /* NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables) */
  extern obj::boolean_ref jank_true;
  /* NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables) */
  extern obj::boolean_ref jank_false;
}
