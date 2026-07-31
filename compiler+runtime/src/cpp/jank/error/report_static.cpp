#include <jtl/terminal.hpp>

#include <jank/error/report.hpp>
#include <jank/util/fmt/print.hpp>

namespace jank::error
{
  void report(error_ref const e)
  {
    util::println(stderr,
                  "{}error:{} {}",
                  jtl::terminal::text_style::red,
                  jtl::terminal::text_style::reset,
                  e->message);
  }
}
