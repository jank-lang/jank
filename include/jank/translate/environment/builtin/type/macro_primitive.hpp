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
          scope& add_macro_primitives(scope &s);

          cell::detail::type_reference<cell::cell> macro_atom(scope &s);
          cell::detail::type_reference<cell::cell> macro_list(scope &s);
        }
      }
    }
  }
}
