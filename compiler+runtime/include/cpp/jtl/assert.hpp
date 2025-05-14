#pragma once

#ifdef JANK_DEBUG
  /* NOLINTNEXTLINE(cppcoreguidelines-macro-usage) */
  #define jank_debug_assert(expr)                          \
    if(!static_cast<bool>(expr))                           \
    {                                                      \
      jtl::do_assertion_panic("Assertion failed! " #expr); \
    }
  /* NOLINTNEXTLINE(cppcoreguidelines-macro-usage) */
  #define jank_debug_assert_fmt(expr, fmt, ...)                        \
    if(!static_cast<bool>(expr))                                       \
    {                                                                  \
      jtl::do_assertion_panic(jank::util::format((fmt), __VA_ARGS__)); \
    }

  /* NOLINTNEXTLINE(cppcoreguidelines-macro-usage) */
  #define jank_debug_assert_throw(expr)                    \
    if(!static_cast<bool>(expr))                           \
    {                                                      \
      jtl::do_assertion_throw("Assertion failed! " #expr); \
    }
  /* NOLINTNEXTLINE(cppcoreguidelines-macro-usage) */
  #define jank_debug_assert_fmt_throw(expr, msg, ...)                  \
    if(!static_cast<bool>(expr))                                       \
    {                                                                  \
      jtl::do_assertion_throw(jank::util::format((fmt), __VA_ARGS__)); \
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
#define jank_assert_fmt(expr, fmt, ...)                              \
  if(!static_cast<bool>(expr))                                       \
  {                                                                  \
    jtl::do_assertion_panic(jank::util::format((fmt), __VA_ARGS__)); \
  }

/* NOLINTNEXTLINE(cppcoreguidelines-macro-usage) */
#define jank_assert_throw(expr)                          \
  if(!static_cast<bool>(expr))                           \
  {                                                      \
    jtl::do_assertion_throw("Assertion failed! " #expr); \
  }
/* NOLINTNEXTLINE(cppcoreguidelines-macro-usage) */
#define jank_assert_fmt_throw(expr, fmt, ...)                        \
  if(!static_cast<bool>(expr))                                       \
  {                                                                  \
    jtl::do_assertion_throw(jank::util::format((fmt), __VA_ARGS__)); \
  }

namespace jtl
{
  struct immutable_string;

  [[noreturn]]
  void do_assertion_panic(immutable_string const &msg);
  [[noreturn]]
  void do_assertion_throw(immutable_string const &msg);
}

/* Everyone using these assertions also needs immutable_string, but we need to include
 * it last, since immutable_string also uses these assertions. */
#include <jtl/immutable_string.hpp>
