#include <cpptrace/basic.hpp>

#include <jtl/assert.hpp>
#include <jank/util/fmt/print.hpp>

namespace jtl
{
  namespace detail
  {
    static void assertion_failed_panic(char const * const msg)
    {
      jank::util::println(stderr, "Assertion failed! {}", msg);
      cpptrace::generate_trace().print();
      ::abort();
    }

    static void assertion_failed_throw(char const * const msg)
    {
      throw std::runtime_error{ jank::util::format("Assertion failed! {}", msg) };
    }
  }

  void do_assertion_panic(bool const expr)
  {
    if(!expr)
    {
      detail::assertion_failed_panic("");
    }
  }

  void do_assertion_panic(bool const expr, char const * const msg)
  {
    if(!expr)
    {
      detail::assertion_failed_panic(msg);
    }
  }

  void do_assertion_throw(bool const expr)
  {
    if(!expr)
    {
      detail::assertion_failed_throw("");
    }
  }

  void do_assertion_throw(bool const expr, char const * const msg)
  {
    if(!expr)
    {
      detail::assertion_failed_throw(msg);
    }
  }
}
