#pragma once

#include <jank/translate/expect/error/type/type.hpp>

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
            struct overload
            { static char const constexpr *description{ "overload" }; };
          }
          using overload = type<detail::overload>;
        }
      }
    }
  }
}
