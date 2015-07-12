#pragma once

#include <jank/translate/environment/scope.hpp>

namespace jank
{
  namespace translate
  {
    namespace plugin
    {
      namespace assertion
      {
        void assertion(std::shared_ptr<environment::scope> const &scope);
      }
    }
  }
}
