#pragma once

#include <jank/interpret/environment/scope.hpp>

namespace jank
{
  namespace interpret
  {
    namespace plugin
    {
      namespace arithmetic
      {
        void bitwise_xor
        (
          std::shared_ptr<translate::environment::scope> const &trans_scope,
          std::shared_ptr<environment::scope> const &int_scope
        );
      }
    }
  }
}
