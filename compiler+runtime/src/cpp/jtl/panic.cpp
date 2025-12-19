#include <cpptrace/basic.hpp>

#include <jtl/panic.hpp>
#include <jank/util/fmt/print.hpp>

namespace jtl::detail
{
  [[noreturn]]
  void panic(char const * const msg)
  {
    jank::util::println(stderr, "Panic encountered: {}", msg);
    cpptrace::generate_trace().print();
    ::abort();
  }
}
