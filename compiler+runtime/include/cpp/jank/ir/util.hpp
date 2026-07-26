#pragma once

#include <jtl/immutable_string.hpp>
#include <jtl/ptr.hpp>

namespace jank::ir
{
  struct function;
  struct instruction;
  using identifier = jtl::immutable_string;

  bool uses_name(jtl::ref<instruction> const inst, identifier const &name);
  void replace_with_nop(function &fn, identifier const &block, identifier const &name);
}
