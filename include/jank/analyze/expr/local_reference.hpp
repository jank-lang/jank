#pragma once

#include <jank/runtime/obj/symbol.hpp>
#include <jank/analyze/frame.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct local_reference
  {
    runtime::obj::symbol_ptr name;
    local_binding<E> const &binding;
  };
}
