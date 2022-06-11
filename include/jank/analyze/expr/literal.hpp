#pragma once

#include <vector>

#include <jank/runtime/object.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct literal
  {
    runtime::object_ptr data;
  };
}
