#pragma once

#include <jank/runtime/obj/symbol.hpp>
#include <jank/analyze/local_frame.hpp>

namespace jank::analyze::expr
{
  struct local_reference
  {
    runtime::obj::symbol_ptr name;
    local_binding const &binding;
  };
}
