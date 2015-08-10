#pragma once

#include <jank/translate/environment/scope.hpp>
#include <jank/translate/cell/detail/function_reference.hpp>

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
          scope& add_function(scope &s);

          cell::detail::function_reference<cell::detail::function_definition<cell::cell>>
          function(scope &s);
        }
      }
    }
  }
}
