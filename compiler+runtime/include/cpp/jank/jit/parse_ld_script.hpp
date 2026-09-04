#pragma once

#include <jtl/immutable_string.hpp>
#include <jtl/option.hpp>

namespace jank::jit
{
  bool is_object_file(jtl::immutable_string const &path);
  jtl::option<jtl::immutable_string> parse_ld_script(jtl::immutable_string const &path);
}
