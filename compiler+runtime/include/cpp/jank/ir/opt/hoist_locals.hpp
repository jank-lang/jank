#pragma once

namespace jank::ir
{
  struct function;

  void hoist_locals(function &fn);
}
