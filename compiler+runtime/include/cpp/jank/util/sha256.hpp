#pragma once

#include <jank/native_persistent_string.hpp>

namespace jank::util
{
  native_persistent_string sha256(native_persistent_string const &input);
}
