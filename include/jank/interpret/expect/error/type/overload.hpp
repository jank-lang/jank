#pragma once

#include <jank/interpret/expect/error/type/type.hpp>

namespace jank
{
  namespace interpret
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
          using overload = exception<detail::overload>;
        }
      }
    }
  }
}
