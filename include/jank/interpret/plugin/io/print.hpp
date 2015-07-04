#pragma once

#include <jank/interpret/environment/scope.hpp>

namespace jank
{
  namespace interpret
  {
    namespace plugin
    {
      namespace io
      {
        /* TODO: move to cpp. */
        inline void print(std::shared_ptr<environment::scope> const &scope)
        {
          static_cast<void>(scope);
          /* TODO: Add the print body; bring in args as a vector. */
        }
      }
    }
  }
}
