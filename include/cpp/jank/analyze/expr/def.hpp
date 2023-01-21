#pragma once

#include <memory>

#include <jank/runtime/obj/symbol.hpp>
#include <jank/analyze/local_frame.hpp>
#include <jank/analyze/expression_base.hpp>
#include <jank/option.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct def : expression_base
  {
    runtime::obj::symbol_ptr name;
    option<native_box<E>> value;
    local_frame_ptr frame;
  };
}
