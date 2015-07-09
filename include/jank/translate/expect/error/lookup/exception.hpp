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
        namespace lookup
        {
          namespace detail
          {
            struct generic
            { static char const constexpr *description{ "generic" }; };
          }

          template <typename T = detail::generic>
          struct exception : std::runtime_error
          {
            exception()
              : std::runtime_error
                { std::string{ "translate lookup error (" } + T::description + ")" }
            { }
            exception(std::string const &s)
              : std::runtime_error
                { std::string{ "translate lookup error (" } + T::description + "): " + s }
            { }
          };
        }
      }
    }
  }
}
