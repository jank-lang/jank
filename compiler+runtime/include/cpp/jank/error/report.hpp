#pragma once

#include <jank/error.hpp>
#include <jtl/immutable_string.hpp>

namespace jank::error
{
  jtl::immutable_string report(error_ref e);
  jtl::immutable_string warn(jtl::immutable_string const &);
}
