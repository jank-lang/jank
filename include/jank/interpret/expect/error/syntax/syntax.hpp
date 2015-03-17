#pragma once

#include <stdexcept>
#include <string>

namespace jank
{
  namespace expect
  {
    namespace error
    {
      namespace syntax
      {
        struct generic
        { static char const constexpr *description{ "expression" }; };

        template <typename T = generic>
        struct syntax : std::runtime_error
        {
          syntax()
            : std::runtime_error
              { std::string{ "syntax error (" } + T::description + ")" }
          { }
          syntax(std::string const &s)
            : std::runtime_error
              { std::string{ "syntax error (" } + T::description + "): " + s }
          { }
        };
      }
    }
  }
}
