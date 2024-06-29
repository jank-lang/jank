#pragma once

#include "jank/type.hpp"

namespace jank::util::character
{
  option<char> get_char_from_repr(native_persistent_string_view const &);
}
