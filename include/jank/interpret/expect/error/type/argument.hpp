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
          struct argument
          { static char const constexpr *description{ "argument" }; };
        }
        using argument = type<detail::argument>;
      }
    }
  }
}
