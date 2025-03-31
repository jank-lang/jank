#pragma once

#ifdef JANK_DEBUG
  /* NOLINTNEXTLINE(cppcoreguidelines-macro-usage) */
  #define jank_debug_assert(expr) jtl::do_debug_assertion(static_cast<bool>(expr));
  /* NOLINTNEXTLINE(cppcoreguidelines-macro-usage) */
  #define jank_debug_assert_msg(expr, msg) jtl::do_debug_assertion(static_cast<bool>(expr), msg);
#else
  /* NOLINTNEXTLINE(cppcoreguidelines-macro-usage) */
  #define jank_debug_assert(expr) static_cast<void>(0)
  /* NOLINTNEXTLINE(cppcoreguidelines-macro-usage) */
  #define jank_debug_assert_msg(expr, msg) static_cast<void>(0)
#endif

/* NOLINTNEXTLINE(cppcoreguidelines-macro-usage) */
#define jank_assert(expr) jtl::do_release_assertion(static_cast<bool>(expr));
/* NOLINTNEXTLINE(cppcoreguidelines-macro-usage) */
#define jank_assert_msg(expr, msg) jtl::do_release_assertion(static_cast<bool>(expr), msg);

namespace jtl
{
  namespace detail
  {
    void assertion_failed(char const *msg);
  }

  void do_debug_assertion(bool const expr);
  void do_debug_assertion(bool const expr, char const * const msg);
}
