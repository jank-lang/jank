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

          /* TODO: Rename these templates to 'exception'. */
          template <typename T = generic>
          struct internal : std::runtime_error
          {
            internal()
              : std::runtime_error
                { std::string{ "internal error (" } + T::description + ")" }
            { }
            internal(std::string const &s)
              : std::runtime_error
                { std::string{ "internal error (" } + T::description + "): " + s }
            { }
          };
        }
      }
    }
  }
}
