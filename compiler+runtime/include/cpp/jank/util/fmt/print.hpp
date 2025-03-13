#pragma once

#include <jank/util/fmt.hpp>

namespace jank::util
{
  void println(char const * const fmt);
  void println(FILE * const file, char const * const fmt);
  void print(char const * const fmt);
  void print(FILE * const file, char const * const fmt);

  template <typename T, typename... Args>
  void println(char const * const fmt, T &&arg, Args &&...args)
  {
    string_builder sb;
    format_to(sb, fmt, std::forward<T>(arg), std::forward<Args>(args)...);
    std::fwrite(sb.data(), 1, sb.size(), stdout);
    std::putc('\n', stdout);
  }

  template <typename T, typename... Args>
  void println(FILE * const file, char const * const fmt, T &&arg, Args &&...args)
  {
    string_builder sb;
    format_to(sb, fmt, std::forward<T>(arg), std::forward<Args>(args)...);
    std::fwrite(sb.data(), 1, sb.size(), file);
    std::putc('\n', file);
  }

  template <typename T, typename... Args>
  void print(char const * const fmt, T &&arg, Args &&...args)
  {
    string_builder sb;
    format_to(sb, fmt, std::forward<T>(arg), std::forward<Args>(args)...);
    std::fwrite(sb.data(), 1, sb.size(), stdout);
  }

  template <typename T, typename... Args>
  void print(FILE * const file, char const * const fmt, T &&arg, Args &&...args)
  {
    string_builder sb;
    format_to(sb, fmt, std::forward<T>(arg), std::forward<Args>(args)...);
    std::fwrite(sb.data(), 1, sb.size(), file);
  }
}
