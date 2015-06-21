#pragma once

#include <memory>
#include <vector>

#include <jank/translate/cell/detail/type_reference.hpp>

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
        /* TODO: Allow native function bodies.
         * 1) function for translating
         * 2) function for interpreting
         * 3) function for streaming
         */
        template <typename C>
        struct function_body
        {
          std::vector<C> cells;
          type_reference return_type;
          std::shared_ptr<environment::scope> scope;
        };
      }
    }
  }
}
