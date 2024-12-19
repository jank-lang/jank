#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using character_ptr = native_box<struct character>;

  struct character : gc
  {
    static constexpr object_type obj_type{ object_type::character };
    static constexpr native_bool pointer_free{ false };

    character() = default;
    character(character &&) noexcept = default;
    character(character const &) = default;
    character(native_persistent_string const &);
    character(char);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    native_persistent_string to_string() const;
    void to_string(util::string_builder &buff) const;
    native_persistent_string to_code_string() const;
    native_hash to_hash() const;

    object base{ obj_type };
    /* Holds the raw form of the character bytes. Supports Unicode. */
    native_persistent_string data;
  };
}
