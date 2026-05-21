#pragma once

namespace jank::ir
{
  struct function;

  void remove_nops(function &fn);
}
