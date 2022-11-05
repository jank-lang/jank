#pragma once

#include <vector>

#include <jank/runtime/object.hpp>
#include <jank/analyze/local_frame.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct primitive_literal
  {
    runtime::object_ptr data;
    local_frame_ptr frame;
  };
}
