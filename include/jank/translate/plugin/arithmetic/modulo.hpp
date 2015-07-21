#pragma once

#include <jank/translate/environment/scope.hpp>

namespace jank
{
  namespace translate
  {
    namespace plugin
    {
      namespace arithmetic
      {
        void modulo(std::shared_ptr<environment::scope> const &scope);
      }
    }
  }
}
