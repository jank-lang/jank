#pragma once

#include <jank/translate/environment/scope.hpp>

namespace jank
{
  namespace translate
  {
    namespace plugin
    {
      namespace io
      {
        inline void print(std::shared_ptr<environment::scope> const &scope)
        {
          static_cast<void>(scope);
        }
      }
    }
  }
}
