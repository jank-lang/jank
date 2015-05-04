#pragma once

#include <ostream>

#include <jank/parse/cell/cell.hpp>

namespace jank
{
  namespace parse
  {
    namespace cell
    {
      std::ostream& operator <<(std::ostream &os, cell const &c);
    }
  }
}
