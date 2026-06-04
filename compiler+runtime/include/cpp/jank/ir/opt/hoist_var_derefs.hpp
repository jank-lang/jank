#pragma once

namespace jank::ir
{
  struct function;

  void hoist_var_derefs(function &fn);
}
