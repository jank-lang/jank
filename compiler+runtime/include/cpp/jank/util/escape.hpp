#pragma once

#include <fmt/base.h>

#include <jank/native_persistent_string.hpp>
#include <jank/result.hpp>

namespace jank::util
{
  /* This provides a fmt extension for escaping strings and wrapping them in
   * quotes. It's largely adapted from here:
   * https://github.com/fmtlib/fmt/issues/825#issuecomment-1227501168
   *
   * Usage just looks like:
   * fmt::format("{}", util::escaped_quoted_view(s))
   */
  template <typename S = native_persistent_string_view>
  struct escape_view
  {
    template <typename It>
    constexpr It copy(It out) const
    {
      *out++ = quote;
      for(auto const c : sv)
      {
        switch(c)
        {
          case '\n':
            *out++ = '\\';
            *out++ = 'n';
            break;
          case '\t':
            *out++ = '\\';
            *out++ = 't';
            break;
          case '\r':
            *out++ = '\\';
            *out++ = 'r';
            break;
          case '\\':
            *out++ = '\\';
            *out++ = '\\';
            break;
          case '"':
            *out++ = '\\';
            *out++ = '"';
            break;
          default:
            *out++ = c;
        }
      }
      *out++ = quote;
      return out;
    }

    S sv;
    typename S::value_type quote{ '"' };
    typename S::value_type esc{ '\\' };
  };

  constexpr escape_view<native_persistent_string_view>
  escaped_quoted_view(native_persistent_string_view const &sv,
                      char const q = '"',
                      char const e = '\\')
  {
    return escape_view<native_persistent_string_view>{ sv, q, e };
  }

  struct unescape_error
  {
    native_persistent_string message;
  };

  /* These provide normal escaping/unescaping, with no quoting. */
  result<native_persistent_string, unescape_error> unescape(native_persistent_string const &input);
  native_persistent_string escape(native_persistent_string const &input);
}

template <typename S>
struct fmt::formatter<jank::util::escape_view<S>>
{
  using V = jank::util::escape_view<S>;

  template <typename C>
  constexpr auto parse(C &ctx)
  {
    return ctx.begin();
  }

  template <typename C>
  auto format(jank::util::escape_view<S> const &s, C &ctx) const
  {
    return s.copy(ctx.out());
  }
};
