#pragma once

#include <jtl/primitive.hpp>
#include <jtl/immutable_string.hpp>

namespace jtl::terminal
{
  enum class text_style : u32
  {
    reset = 0,
    bold = 1 << 0,
    underline = 1 << 1,
    no_underline = 1 << 2,
    black = 1 << 3,
    red = 1 << 4,
    green = 1 << 5,
    yellow = 1 << 6,
    blue = 1 << 7,
    magenta = 1 << 8,
    cyan = 1 << 9,
    white = 1 << 10,
    bright_black = 1 << 11,
    bright_red = 1 << 12,
    bright_green = 1 << 13,
    bright_yellow = 1 << 14,
    bright_blue = 1 << 15,
    bright_magenta = 1 << 16,
    bright_cyan = 1 << 17,
    bright_white = 1 << 18,
  };

  constexpr text_style operator|(text_style const a, text_style const b)
  {
    using U = std::underlying_type_t<text_style>;
    return static_cast<text_style>(static_cast<U>(a) | static_cast<U>(b));
  }

  constexpr text_style operator&(text_style const a, text_style const b)
  {
    using U = std::underlying_type_t<text_style>;
    return static_cast<text_style>(static_cast<U>(a) & static_cast<U>(b));
  }

  constexpr text_style &operator|=(text_style &a, text_style const b)
  {
    return a = a | b;
  }

  constexpr bool has_flag(text_style const s, text_style const flag)
  {
    using U = std::underlying_type_t<text_style>;
    return flag != text_style::reset
      && (static_cast<U>(s) & static_cast<U>(flag)) == static_cast<U>(flag);
  }

  static constexpr std::pair<text_style, char const *> color_codes[]{
    {          text_style::black, "\u001b[30m" },
    {            text_style::red, "\u001b[31m" },
    {          text_style::green, "\u001b[32m" },
    {         text_style::yellow, "\u001b[33m" },
    {           text_style::blue, "\u001b[34m" },
    {        text_style::magenta, "\u001b[35m" },
    {           text_style::cyan, "\u001b[36m" },
    {          text_style::white, "\u001b[37m" },
    {   text_style::bright_black, "\u001b[90m" },
    {     text_style::bright_red, "\u001b[91m" },
    {   text_style::bright_green, "\u001b[92m" },
    {  text_style::bright_yellow, "\u001b[93m" },
    {    text_style::bright_blue, "\u001b[94m" },
    { text_style::bright_magenta, "\u001b[95m" },
    {    text_style::bright_cyan, "\u001b[96m" },
    {   text_style::bright_white, "\u001b[97m" },
  };

  struct size
  {
    usize width{};
    usize height{};
  };

  size get_size();
  bool is_interactive();
}
