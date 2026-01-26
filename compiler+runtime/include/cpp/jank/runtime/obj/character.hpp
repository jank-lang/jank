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

    /* behavior::object_like */
    bool equal(object const &) const override;
    jtl::immutable_string to_string() const override;
    void to_string(jtl::string_builder &buff) const override;
    jtl::immutable_string to_code_string() const override;
    uhash to_hash() const override;

    /*** XXX: Everything here is immutable after initialization. ***/

    /* Holds the raw form of the character bytes. Supports Unicode. */
    jtl::immutable_string data;
  };
}
