#pragma once

#ifdef JANK_DEBUG
  /* NOLINTNEXTLINE(cppcoreguidelines-macro-usage) */
  #define jank_debug_assert(expr)                          \
    if(!static_cast<bool>(expr))                           \
    {                                                      \
      jtl::do_assertion_panic("Assertion failed! " #expr); \
    }
  /* NOLINTNEXTLINE(cppcoreguidelines-macro-usage) */
  #define jank_debug_assert_fmt(expr, fmt, ...)                               \
    if(!static_cast<bool>(expr))                                              \
    {                                                                         \
      jtl::do_assertion_panic(jank::util::format((fmt), __VA_ARGS__).view()); \
    }

  /* NOLINTNEXTLINE(cppcoreguidelines-macro-usage) */
  #define jank_debug_assert_throw(expr)                    \
    if(!static_cast<bool>(expr))                           \
    {                                                      \
      jtl::do_assertion_throw("Assertion failed! " #expr); \
    }
  /* NOLINTNEXTLINE(cppcoreguidelines-macro-usage) */
  #define jank_debug_assert_fmt_throw(expr, fmt, ...)                         \
    if(!static_cast<bool>(expr))                                              \
    {                                                                         \
      jtl::do_assertion_throw(jank::util::format((fmt), __VA_ARGS__).view()); \
    }
#else
  /* NOLINTNEXTLINE(cppcoreguidelines-macro-usage) */
  #define jank_debug_assert(expr) static_cast<void>(0)
  /* NOLINTNEXTLINE(cppcoreguidelines-macro-usage) */
  #define jank_debug_assert_fmt(expr, fmt, ...) static_cast<void>(0)

  /* NOLINTNEXTLINE(cppcoreguidelines-macro-usage) */
  #define jank_debug_assert_throw(expr) static_cast<void>(0)
  /* NOLINTNEXTLINE(cppcoreguidelines-macro-usage) */
  #define jank_debug_assert_fmt_throw(expr, fmt, ...) static_cast<void>(0)
#endif

/* NOLINTNEXTLINE(cppcoreguidelines-macro-usage) */
#define jank_assert(expr)                                \
  if(!static_cast<bool>(expr))                           \
  {                                                      \
    jtl::do_assertion_panic("Assertion failed! " #expr); \
  }
/* NOLINTNEXTLINE(cppcoreguidelines-macro-usage) */
#define jank_assert_fmt(expr, fmt, ...)                                     \
  if(!static_cast<bool>(expr))                                              \
  {                                                                         \
    jtl::do_assertion_panic(jank::util::format((fmt), __VA_ARGS__).view()); \
  }

/* NOLINTNEXTLINE(cppcoreguidelines-macro-usage) */
#define jank_assert_throw(expr)                          \
  if(!static_cast<bool>(expr))                           \
  {                                                      \
    jtl::do_assertion_throw("Assertion failed! " #expr); \
  }
/* NOLINTNEXTLINE(cppcoreguidelines-macro-usage) */
#define jank_assert_fmt_throw(expr, fmt, ...)                               \
  if(!static_cast<bool>(expr))                                              \
  {                                                                         \
    jtl::do_assertion_throw(jank::util::format((fmt), __VA_ARGS__).view()); \
  }

namespace jtl
{
  struct immutable_string_view;

  [[noreturn]]
  void do_assertion_panic(immutable_string_view const &msg);
  [[noreturn]]
  void do_assertion_throw(immutable_string_view const &msg);
}

/* Everyone using these assertions also needs immutable_string_view, but we need to include
 * it last, since immutable_string_view also uses these assertions. */
#include <jtl/immutable_string.hpp>
