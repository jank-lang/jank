#pragma once

#include <jtl/immutable_string.hpp>
#include <jtl/ptr.hpp>

namespace jank::ir
{
  struct function;
  struct instruction;
  using identifier = jtl::immutable_string;

  bool rewrite_uses(jtl::ref<instruction> const inst,
                    identifier const &old_name,
                    identifier const &new_name);
  void rewrite_uses(function const &fn, identifier const &old_name, identifier const &new_name);
}
