#pragma once

/* This provides a fmt extension for escaping strings and wrapping them in
 * quotes. It's largely adapted from here:
 * https://github.com/fmtlib/fmt/issues/825#issuecomment-1227501168 */
namespace jank::codegen
{
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
  escaped(native_persistent_string_view const &sv, char const q = '"', char const e = '\\')
  {
    return escape_view<native_persistent_string_view>{ sv, q, e };
  }
}

template <typename S>
struct fmt::formatter<jank::codegen::escape_view<S>>
{
  using V = jank::codegen::escape_view<S>;

  template <typename C>
  constexpr auto parse(C &ctx)
  {
    return ctx.begin();
  }

  template <typename C>
  auto format(jank::codegen::escape_view<S> const &s, C &ctx)
  {
    return s.copy(ctx.out());
  }
};
