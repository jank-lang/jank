#pragma once

#include <jtl/primitive.hpp>

namespace jtl
{
  enum class terminal_style : u8
  {
    reset = 0,
    bold = 1,
    no_bold = 22,
    italic = 3,
    no_italic = 23,
    underline = 4,
    no_underline = 24,
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
