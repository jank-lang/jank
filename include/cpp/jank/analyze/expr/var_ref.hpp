#pragma once

#include <jank/runtime/var.hpp>
#include <jank/analyze/local_frame.hpp>
#include <jank/analyze/expression_base.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct var_ref : expression_base
  {
    runtime::obj::symbol_ptr qualified_name{};
    local_frame_ptr frame{};
    runtime::var_ptr var{};
  };
}
