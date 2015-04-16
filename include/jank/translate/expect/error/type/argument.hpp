#pragma once

#include <jank/translate/expect/error/type/type.hpp>

/* TODO remove? */

namespace jank
{
  namespace translate
  {
    namespace expect
    {
      namespace error
      {
        namespace type
        {
          namespace detail
          {
            struct argument
            { static char const constexpr *description{ "argument" }; };
          }
          using argument = type<detail::argument>;
        }
      }
    }
  }
}
