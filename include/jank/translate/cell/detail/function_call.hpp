#pragma once

#include <string>

namespace jank
{
  namespace translate
  {
    namespace cell
    {
      namespace detail
      {
        template <typename C>
        struct function_call
        {
          /* TODO: argument list. */
          std::string name;
        };
      }
    }
  }
}
