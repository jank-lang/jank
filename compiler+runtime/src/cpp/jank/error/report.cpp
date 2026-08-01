#include <jtl/terminal.hpp>

#include <jank/error/report.hpp>
#include <jank/util/fmt/print.hpp>

namespace jank::error
{
  void warn(jtl::immutable_string const &msg)
  {
    util::println(stderr,
                  "{}warning:{} {}",
                  jtl::terminal::text_style::yellow,
                  jtl::terminal::text_style::reset,
                  msg);
  }
}
