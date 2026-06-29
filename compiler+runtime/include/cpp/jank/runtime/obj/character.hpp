#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using character_ref = oref<struct character>;

  struct character : object
  {
    static constexpr object_type obj_type{ object_type::character };
    static constexpr object_behavior obj_behaviors{ object_behavior::none };
    static constexpr bool pointer_free{ false };

    character();
    character(character &&) noexcept = default;
    character(character const &) = default;
    character(jtl::immutable_string const &);
    character(char);
    character(i64);

    /* behavior::object_like */
    bool equal(object const &) const override;
    jtl::immutable_string to_string() const override;
    void to_string(jtl::string_builder &buff) const override;
    jtl::immutable_string to_code_string() const override;
    uhash to_hash() const override;

    /* behavior::comparable */
    i64 compare(object const &) const override;

    /* behavior::comparable extended */
    i64 compare(character const &) const;

    /* Character does not fully support `behavior::number_like`, but can be converted to an integer. */
    i64 to_integer() const override;

    /*** XXX: Everything here is immutable after initialization. ***/

    /* Holds the raw form of the character bytes. Supports Unicode. */
    jtl::immutable_string data;
  };
}
