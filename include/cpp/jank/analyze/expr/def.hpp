#pragma once

#include <memory>

#include <jank/runtime/obj/symbol.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct def
  {
    runtime::obj::symbol_ptr name;
    std::shared_ptr<E> value;
    /* TODO: Keep track of whether this is a redefinition. Maybe keep the existing var. */
  };
}
