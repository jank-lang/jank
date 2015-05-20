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
        namespace internal
        {
          struct generic
          { static char const constexpr *description{ "expression" }; };

          template <typename T = generic>
          struct exception : std::runtime_error
          {
            exception()
              : std::runtime_error
                { std::string{ "internal error (" } + T::description + ")" }
            { }
            exception(std::string const &s)
              : std::runtime_error
                { std::string{ "internal error (" } + T::description + "): " + s }
            { }
          };
        }
      }
    }
  }
}
