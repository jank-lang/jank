#pragma once

#include <jank/expect/error/type/type.hpp>

namespace jank
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
