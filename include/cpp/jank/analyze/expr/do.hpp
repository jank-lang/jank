#pragma once

#include <list>

namespace jank::analyze::expr
{
  template <typename E>
  struct do_
  {
    /* Stable references. */
    std::list<E> body;
  };
}
