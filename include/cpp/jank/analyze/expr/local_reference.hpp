#pragma once

#include <jank/runtime/obj/symbol.hpp>
#include <jank/analyze/local_frame.hpp>
#include <jank/analyze/expression_base.hpp>

namespace jank::analyze::expr
{
  struct local_reference : expression_base
  {
    runtime::obj::symbol_ptr name{};
    local_binding const &binding;
  };
}
