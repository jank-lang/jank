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
          /* TODO: Add native function impl to scope. What about overloads? */
        }
      }
    }
  }
}
