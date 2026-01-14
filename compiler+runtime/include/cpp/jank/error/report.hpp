#pragma once

#include <jank/error.hpp>

namespace jank::error
{
  void report(error_ref e);
  void warn(jtl::immutable_string const &);
}
