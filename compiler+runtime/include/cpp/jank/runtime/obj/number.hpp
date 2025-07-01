#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using boolean_ref = oref<struct boolean>;

  struct boolean : gc
  {
    static constexpr object_type obj_type{ object_type::boolean };
    static constexpr bool pointer_free{ true };

    boolean() = default;
    boolean(boolean &&) noexcept = default;
    boolean(boolean const &) = default;
    boolean(bool const d);

    /* behavior::object_like */
    bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    void to_string(util::string_builder &buff) const;
    jtl::immutable_string to_code_string() const;
    uhash to_hash() const;

    /* behavior::comparable */
    i64 compare(object const &) const;

    /* behavior::comparable extended */
    i64 compare(boolean const &) const;

    object base{ obj_type };
    bool data{};
  };

  using integer_ref = oref<struct integer>;

  struct integer : gc
  {
    static constexpr object_type obj_type{ object_type::integer };
    static constexpr bool pointer_free{ true };

    integer() = default;
    integer(integer &&) noexcept = default;
    integer(integer const &) = default;
    integer(i64 const d);

    /* behavior::object_like */
    bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    void to_string(util::string_builder &buff) const;
    jtl::immutable_string to_code_string() const;
    uhash to_hash() const;

    /* behavior::comparable */
    i64 compare(object const &) const;

    /* behavior::comparable extended */
    i64 compare(integer const &) const;

    /* behavior::number_like */
    i64 to_integer() const;
    f64 to_real() const;

    object base{ obj_type };
    /* TODO: Is it faster to have the data first or the base first? */
    i64 data{};
  };

  using real_ref = oref<struct real>;

  struct real : gc
  {
    static constexpr object_type obj_type{ object_type::real };
    static constexpr bool pointer_free{ true };

    real() = default;
    real(real &&) noexcept = default;
    real(real const &) = default;
    real(f64 const d);

    /* behavior::object_like */
    bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    void to_string(util::string_builder &buff) const;
    jtl::immutable_string to_code_string() const;
    uhash to_hash() const;

    /* behavior::comparable */
    i64 compare(object const &) const;

    /* behavior::comparable extended */
    i64 compare(real const &) const;

    /* behavior::number_like */
    i64 to_integer() const;
    f64 to_real() const;

    f64 data{};
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
