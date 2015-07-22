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
        void bitwise_left_shift(std::shared_ptr<environment::scope> const &scope);
      }
    }
  }
}
