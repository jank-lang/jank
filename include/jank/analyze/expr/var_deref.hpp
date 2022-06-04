#pragma once

#include <jank/runtime/var.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct var_deref
  {
    runtime::var_ptr var;
  };
}
