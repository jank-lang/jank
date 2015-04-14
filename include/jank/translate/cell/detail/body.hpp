#pragma once

#include <vector>

namespace jank
{
  namespace translate
  {
    namespace cell
    {
      namespace detail
      {
        template <typename C>
        struct body
        {
          std::vector<C> cells;
        };
      }
    }
  }
}
