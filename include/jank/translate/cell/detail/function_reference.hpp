#pragma once

#include <jank/translate/cell/detail/function_definition.hpp>

namespace jank
{
  namespace translate
  {
    namespace cell
    {
      namespace detail
      {
        /* May be native or non-native. */
        template <typename Def>
        struct function_reference
        {
          Def definition;
        };
      }
    }
  }
}
