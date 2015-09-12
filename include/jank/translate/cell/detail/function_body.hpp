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
        template <typename C>
        struct function_body
        {
          std::vector<C> cells;
          type_reference<C> return_type;
          std::shared_ptr<environment::scope> scope;
        };
      }
    }
  }
}
