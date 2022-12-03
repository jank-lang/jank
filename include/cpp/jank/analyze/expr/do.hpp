#pragma once

#include <vector>

namespace jank::analyze::expr
{
  template <typename E>
  struct do_
  { std::vector<std::shared_ptr<E>> body; };
}
