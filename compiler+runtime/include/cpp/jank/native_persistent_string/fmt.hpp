#pragma once

#include <fmt/base.h>

#include <jank/native_persistent_string.hpp>

template <>
struct fmt::formatter<jank::native_persistent_string> : private formatter<fmt::string_view>
{
  using formatter<fmt::string_view>::parse;

  template <typename Context>
  auto format(jank::native_persistent_string const &s, Context &ctx) const
  {
    return formatter<fmt::string_view>::format({ s.data(), s.size() }, ctx);
  }
};
