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
        namespace syntax
        {
          namespace detail
          {
            struct generic
            { static char const constexpr *description{ "expression" }; };
          }

          template <typename T = detail::generic>
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
}
