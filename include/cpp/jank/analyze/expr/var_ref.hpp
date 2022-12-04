#pragma once

#include <jank/runtime/var.hpp>
#include <jank/analyze/local_frame.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct var_ref
  {
    runtime::obj::symbol_ptr qualified_name;
    std::shared_ptr<E> var;
    local_frame_ptr frame;
  };
}
