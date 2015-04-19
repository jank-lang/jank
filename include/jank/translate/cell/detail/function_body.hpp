#pragma once

#include <memory>
#include <vector>

namespace jank
{
  namespace translate
  {
    namespace environment
    { struct scope; }

    namespace cell
    {
      namespace detail
      {
        template <typename C>
        struct function_body
        {
          std::vector<C> cells;
          std::shared_ptr<environment::scope> scope;
        };
      }
    }
  }
}
