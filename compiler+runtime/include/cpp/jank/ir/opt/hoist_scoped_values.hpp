#pragma once

namespace jank::ir
{
  struct function;

  void hoist_scoped_values(function &fn);
}
