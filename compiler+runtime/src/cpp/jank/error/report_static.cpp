#include <jtl/format/style.hpp>

#include <jank/error/report.hpp>
#include <jank/util/fmt/print.hpp>

namespace jank::error
{
  void report(error_ref const e)
  {
    util::println(stderr,
                  "{}error:{} {}",
                  jtl::terminal_style::red,
                  jtl::terminal_style::reset,
                  e->message);
  }
}
