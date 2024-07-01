#pragma once

#include <jank/option.hpp>

namespace jank::util::character
{
  option<char> get_char_from_repr(native_persistent_string const &);
}
