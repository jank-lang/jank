#include <jtl/terminal.hpp>

#include <jank/error/report.hpp>
#include <jank/util/fmt/print.hpp>

namespace jank::error
{
  jtl::immutable_string warn(jtl::immutable_string const &msg)
  {
    return util::format("{}warning:{} {}\n",
                        jtl::terminal::text_style::yellow,
                        jtl::terminal::text_style::reset,
                        msg);
  }
}
