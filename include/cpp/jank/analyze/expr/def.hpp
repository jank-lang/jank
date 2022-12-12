#pragma once

#include <memory>

#include <jank/runtime/obj/symbol.hpp>
#include <jank/analyze/local_frame.hpp>
#include <jank/option.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct def
  {
    runtime::obj::symbol_ptr name;
    option<std::shared_ptr<E>> value;
    local_frame_ptr frame;
  };
}
