#pragma once

#include <jtl/immutable_string.hpp>

namespace jank::util
{
  jtl::immutable_string sha256(jtl::immutable_string const &input);
}
