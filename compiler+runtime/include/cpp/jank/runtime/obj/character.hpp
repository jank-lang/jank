#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using character_ref = oref<struct character>;

  struct character : gc
  {
    static constexpr object_type obj_type{ object_type::character };
    static constexpr bool pointer_free{ false };

    character() = default;
    character(character &&) noexcept = default;
    character(character const &) = default;
    character(jtl::immutable_string const &);
    character(char);

    /* behavior::object_like */
    bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    void to_string(util::string_builder &buff) const;
    jtl::immutable_string to_code_string() const;
    native_hash to_hash() const;

    object base{ obj_type };
    /* Holds the raw form of the character bytes. Supports Unicode. */
    jtl::immutable_string data;
  };
}
