#pragma once

#include <jank/analyze/local_frame.hpp>
#include <jank/analyze/expression_base.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct primitive_literal : expression_base
  {
    runtime::object_ptr data;
    local_frame_ptr frame;
  };
}
