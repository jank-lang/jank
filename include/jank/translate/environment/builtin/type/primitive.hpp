#pragma once

#include <jank/translate/environment/scope.hpp>
#include <jank/translate/cell/detail/type_reference.hpp>

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

          cell::detail::type_reference automatic(scope &s);
          cell::detail::type_reference null(scope &s);
          cell::detail::type_reference boolean(scope &s);
          cell::detail::type_reference integer(scope &s);
          cell::detail::type_reference real(scope &s);
          cell::detail::type_reference string(scope &s);
        }
      }
    }
  }
}
