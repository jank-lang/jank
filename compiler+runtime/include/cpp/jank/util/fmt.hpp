#pragma once

#include <jank/util/string_builder.hpp>

namespace jank::util
{
  native_persistent_string format(char const * const fmt);
  void format(string_builder &sb, char const * const fmt);

  /* TODO: We can extern template all common usages here, if they show up in the trace. */
  template <typename T>
  void format(string_builder &sb, char const *&fmt, T &&arg)
  {
    for(; *fmt != '\0'; ++fmt)
    {
      if(*fmt == '{' && *(fmt + 1) == '}')
      {
        sb(std::forward<T>(arg));
        fmt += 2;
        return;
      }
      sb(*fmt);
    }
  }

  template <typename... Args>
  native_persistent_string format(char const *fmt, Args &&...args)
  {
    string_builder sb;
    [[maybe_unused]]
    int const dummy[sizeof...(Args)]{ (format(sb, fmt, std::forward<Args>(args)), 0)... };
    sb(fmt);
    return sb.release();
  }

  template <typename... Args>
  void format_to(string_builder &sb, char const *fmt, Args &&...args)
  {
    [[maybe_unused]]
    int const dummy[sizeof...(Args)]{ (format(sb, fmt, std::forward<Args>(args)), 0)... };
    sb(fmt);
  }
}
