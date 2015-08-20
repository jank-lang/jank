#pragma once

#include <ostream>

#include <jank/interpret/cell/cell.hpp>

namespace jank
{
  namespace interpret
  {
    namespace cell
    {
      std::ostream& operator <<(std::ostream &os, cell const &c);
    }
  }
}
