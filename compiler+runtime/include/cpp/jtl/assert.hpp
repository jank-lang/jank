#pragma once

#ifdef JANK_DEBUG
  /* NOLINTNEXTLINE(cppcoreguidelines-macro-usage) */
  #define jank_debug_assert(expr) jtl::do_assertion_panic(static_cast<bool>(expr));
  /* NOLINTNEXTLINE(cppcoreguidelines-macro-usage) */
  #define jank_debug_assert_msg(expr, msg) jtl::do_assertion_panic(static_cast<bool>(expr), msg);

  /* NOLINTNEXTLINE(cppcoreguidelines-macro-usage) */
  #define jank_debug_assert_throw(expr) jtl::do_assertion_throw(static_cast<bool>(expr));
  /* NOLINTNEXTLINE(cppcoreguidelines-macro-usage) */
  #define jank_debug_assert_msg_throw(expr, msg) \
    jtl::do_assertion_throw(static_cast<bool>(expr), msg);
#else
  /* NOLINTNEXTLINE(cppcoreguidelines-macro-usage) */
  #define jank_debug_assert(expr) static_cast<void>(0)
  /* NOLINTNEXTLINE(cppcoreguidelines-macro-usage) */
  #define jank_debug_assert_msg(expr, msg) static_cast<void>(0)

  /* NOLINTNEXTLINE(cppcoreguidelines-macro-usage) */
  #define jank_debug_assert_throw(expr) static_cast<void>(0)
  /* NOLINTNEXTLINE(cppcoreguidelines-macro-usage) */
  #define jank_debug_assert_msg_throw(expr, msg) static_cast<void>(0)
#endif

/* NOLINTNEXTLINE(cppcoreguidelines-macro-usage) */
#define jank_assert(expr) jtl::do_assertion_panic(static_cast<bool>(expr));
/* NOLINTNEXTLINE(cppcoreguidelines-macro-usage) */
#define jank_assert_msg(expr, msg) jtl::do_assertion_panic(static_cast<bool>(expr), msg);

/* NOLINTNEXTLINE(cppcoreguidelines-macro-usage) */
#define jank_assert_throw(expr) jtl::do_assertion_throw(static_cast<bool>(expr));
/* NOLINTNEXTLINE(cppcoreguidelines-macro-usage) */
#define jank_assert_msg_throw(expr, msg) jtl::do_assertion_throw(static_cast<bool>(expr), msg);

namespace jtl
{
  void do_assertion_panic(bool const expr);
  void do_assertion_panic(bool const expr, char const * const msg);
  void do_assertion_throw(bool const expr);
  void do_assertion_throw(bool const expr, char const * const msg);
}
