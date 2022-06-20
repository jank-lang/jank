#pragma once

#include <list>

#include <jank/analyze/frame.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct function
  {
    std::list<E> body;
    frame<E> local_frame;
  };
}
