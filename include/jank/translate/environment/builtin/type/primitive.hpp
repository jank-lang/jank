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
          scope& add_primitives(scope &s);

          cell::detail::type_reference<cell::cell> automatic(scope &s);
          cell::detail::type_reference<cell::cell> null(scope &s);
          cell::detail::type_reference<cell::cell> boolean(scope &s);
          cell::detail::type_reference<cell::cell> integer(scope &s);
          cell::detail::type_reference<cell::cell> real(scope &s);
          cell::detail::type_reference<cell::cell> string(scope &s);
        }
      }
    }
  }
}
