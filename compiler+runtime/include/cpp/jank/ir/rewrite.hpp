#pragma once

#include <jtl/immutable_string.hpp>

namespace jank::ir
{
  struct function;
  using identifier = jtl::immutable_string;

  void rewrite_uses(function const &fn, identifier const &old_name, identifier const &new_name);
}
