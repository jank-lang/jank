#pragma once

#include <vector>

#include <jank/runtime/object.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct primitive_literal
  {
    runtime::object_ptr data;
  };
}
