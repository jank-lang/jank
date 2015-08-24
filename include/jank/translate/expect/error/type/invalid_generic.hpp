#pragma once

#include <jank/translate/expect/error/type/exception.hpp>

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
            struct invalid_generic
            { static char const constexpr *description{ "invalid_generic" }; };
          }
          using invalid_generic = exception<detail::invalid_generic>;
        }
      }
    }
  }
}
