#include <cpptrace/basic.hpp>

#include <jtl/assert.hpp>
#include <jank/util/fmt/print.hpp>

namespace jtl
{
  void do_assertion_panic(immutable_string const &msg)
  {
    jank::util::println(stderr, "{}", msg);
    cpptrace::generate_trace().print();
    ::abort();
  }

  void do_assertion_throw(immutable_string const &msg)
  {
    throw std::runtime_error{ msg };
  }
}
