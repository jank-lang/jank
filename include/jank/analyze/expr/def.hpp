#pragma once

#include <jank/runtime/obj/symbol.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct def
  {
    runtime::obj::symbol_ptr name;
    runtime::object_ptr value;
    /* TODO: Keep track of whether this is a redefinition. Maybe keep the existing var. */
  };
}
