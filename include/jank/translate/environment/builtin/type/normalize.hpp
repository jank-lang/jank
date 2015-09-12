#pragma once

#include <jank/translate/environment/scope.hpp>
#include <jank/translate/cell/detail/type_definition.hpp>

namespace jank
{
  namespace translate
  {
    namespace environment
    {
      namespace builtin
      {
        namespace type
        {
          cell::detail::type_definition<cell::cell> normalize
          (cell::detail::type_definition<cell::cell> type, scope &s);
        }
      }
    }
  }
}
