#pragma once

#include <jank/interpret/expect/error/internal/exception.hpp>

namespace jank
{
  namespace interpret
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
