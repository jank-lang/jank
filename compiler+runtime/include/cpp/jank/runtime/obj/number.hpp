#pragma once

#define JANK_JUST_OBJECT_PLEASE
#include <jank/runtime/object.hpp>
#undef JANK_JUST_OBJECT_PLEASE

namespace jank::runtime::obj
{
  struct boolean : object
  {
    static constexpr object_type obj_type{ object_type::boolean };
    static constexpr object_behavior obj_behaviors{ object_behavior::compare };
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
    i64 compare(object const &) const override;

    /* behavior::comparable extended */
    i64 compare(boolean const &) const;

    /* behavior::number_like */
    i64 to_integer() const;
    f64 to_real() const;

    /*** XXX: Everything here is immutable after initialization. ***/
    bool data{};
  };

  struct integer : object
  {
    static constexpr object_type obj_type{ object_type::integer };
    static constexpr object_behavior obj_behaviors{ object_behavior::compare };
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
    i64 compare(object const &) const override;

    /* behavior::comparable extended */
    i64 compare(integer const &) const;

    /* behavior::number_like */
    i64 to_integer() const;
    f64 to_real() const;

    /*** XXX: Everything here is immutable after initialization. ***/
    i64 data{};
  };

  struct small_integer : object
  {
    static constexpr object_type obj_type{ object_type::small_integer };
    static constexpr object_behavior obj_behaviors{ object_behavior::compare };
    static constexpr bool pointer_free{ true };

    small_integer();
    small_integer(small_integer &&) noexcept = default;
    small_integer(small_integer const &) = default;
    small_integer(i64 const d);

    /* behavior::object_like */
    bool equal(object const &) const override;
    jtl::immutable_string to_string() const override;
    void to_string(jtl::string_builder &buff) const override;
    jtl::immutable_string to_code_string() const override;
    uhash to_hash() const override;

    /* behavior::comparable */
    i64 compare(object const &) const override;

    /* behavior::comparable extended */
    i64 compare(small_integer const &) const;

    /* behavior::number_like */
    i64 to_integer() const;
    f64 to_real() const;

    /*** XXX: Everything here is immutable after initialization. ***/
    i64 data{};
  };

  struct real : object
  {
    static constexpr object_type obj_type{ object_type::real };
    static constexpr object_behavior obj_behaviors{ object_behavior::compare };
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
    i64 compare(object const &) const override;

    /* behavior::comparable extended */
    i64 compare(real const &) const;

    /* behavior::number_like */
    i64 to_integer() const;
    f64 to_real() const;

    /*** XXX: Everything here is immutable after initialization. ***/
    f64 data{};
  };
}
