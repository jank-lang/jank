#pragma once

#include <jank/translate/expect/error/internal/exception.hpp>

namespace jank
{
  namespace translate
  {
    namespace expect
    {
      namespace error
      {
        namespace internal
        {
          namespace detail
          {
            struct unimplemented
            { static char const constexpr *description{ "unimplemented" }; };
          }
          using unimplemented = exception<detail::unimplemented>;
        }
      }
    }
  }
}
