#include <cpptrace/basic.hpp>

#include <jtl/assert.hpp>
#include <jank/util/fmt/print.hpp>

namespace jtl
{
  namespace detail
  {
    void assertion_failed(char const * const msg)
    {
      jank::util::println(stderr, "Assertion failed! {}", msg);
      cpptrace::generate_trace().print();
      ::abort();
    }
  }

  void do_assertion(bool const expr)
  {
    if(!expr)
    {
      detail::assertion_failed("Assertion failed!");
    }
  }

  void do_assertion(bool const expr, char const * const msg)
  {
    if(!expr)
    {
      detail::assertion_failed(msg);
    }
  }
}
