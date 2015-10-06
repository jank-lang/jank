#pragma once

#include <jank/parse/cell/cell.hpp>

namespace jank
{
  namespace translate
  {
    namespace macro
    {
      struct emit_state
      {
        emit_state();
        ~emit_state();

        std::vector<parse::cell::cell> cells;
      };

      void emit(parse::cell::cell const &cell);
    }
  }
}
