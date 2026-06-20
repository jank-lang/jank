#include <jtl/format/style.hpp>

#include <jank/error/report.hpp>
#include <jank/util/fmt/print.hpp>

namespace jank::error
{
  void warn(jtl::immutable_string const &msg)
  {
    util::println(stderr,
                  "{}warning:{} {}",
                  jtl::terminal_style::yellow,
                  jtl::terminal_style::reset,
                  msg);
  }
}
