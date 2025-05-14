#pragma once

#include <jtl/immutable_string.hpp>
#include <jtl/result.hpp>

namespace jank::util
{
  struct format_failure
  {
    jtl::immutable_string reason;
  };

  jtl::result<jtl::immutable_string, format_failure>
  format_cpp_source(jtl::immutable_string const &);
}
