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
        void print(std::shared_ptr<environment::scope> const &scope);
      }
    }
  }
}
