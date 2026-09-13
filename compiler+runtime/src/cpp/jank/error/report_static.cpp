#include <jtl/terminal.hpp>

#include <jank/error/report.hpp>
#include <jank/util/fmt/print.hpp>

namespace jank::error
{
  jtl::immutable_string report(error_ref const e)
  {
    return util::format("{}error:{} {}\n",
                        jtl::terminal::text_style::red,
                        jtl::terminal::text_style::reset,
                        e->message);
  }
}
