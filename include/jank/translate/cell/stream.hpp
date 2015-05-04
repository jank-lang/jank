#pragma once

#include <ostream>

#include <jank/translate/cell/cell.hpp>

namespace jank
{
  namespace translate
  {
    namespace cell
    {
      std::ostream& operator <<(std::ostream &os, cell const &c);
    }
  }
}
