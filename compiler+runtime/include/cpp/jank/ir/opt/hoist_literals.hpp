#pragma once

namespace jank::ir
{
  struct module;
  struct function;

  void hoist_literals(module &mod, function &fn);
}
