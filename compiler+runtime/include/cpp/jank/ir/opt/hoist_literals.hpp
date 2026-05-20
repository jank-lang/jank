#pragma once

namespace jank::ir
{
  struct function;

  void hoist_literals(function &fn);
}
