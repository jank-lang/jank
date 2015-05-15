#pragma once

#include <stdexcept>
#include <string>

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
            struct generic
            { static char const constexpr *description{ "expression" }; };
          }

          /* TODO: Rename to exception. */
          template <typename T = detail::generic>
          struct type : std::runtime_error
          {
            type()
              : std::runtime_error
                { std::string{ "type error (" } + T::description + ")" }
            { }
            type(std::string const &s)
              : std::runtime_error
                { std::string{ "type error (" } + T::description + "): " + s }
            { }
          };
        }
      }
    }
  }
}
