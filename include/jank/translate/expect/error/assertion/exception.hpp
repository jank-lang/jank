#pragma once

#include <stdexcept>
#include <string>

/* TODO: Move to interpret. */
namespace jank
{
  namespace translate
  {
    namespace expect
    {
      namespace error
      {
        namespace assertion
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
                { std::string{ "assertion error (" } + T::description + ")" }
            { }
            exception(std::string const &s)
              : std::runtime_error
                { std::string{ "assertion error (" } + T::description + "): " + s }
            { }
          };
        }
      }
    }
  }
}
