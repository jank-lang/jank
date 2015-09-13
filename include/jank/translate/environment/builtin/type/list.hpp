#pragma once

#include <jank/translate/environment/scope.hpp>

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
          scope& add_list(scope &s);

          cell::detail::type_reference<cell::cell> list(scope &s);
        }
      }
    }
  }
}
