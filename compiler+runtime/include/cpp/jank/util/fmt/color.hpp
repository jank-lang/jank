#pragma once

#include <jtl/primitive.hpp>

namespace jank::util
{
  enum class terminal_color : u8
  {
    reset = 0,
    black = 30,
    red,
    green,
    yellow,
    blue,
    magenta,
    cyan,
    white,
    bright_black = 90,
    bright_red,
    bright_green,
    bright_yellow,
    bright_blue,
    bright_magenta,
    bright_cyan,
    bright_white
  };
}
