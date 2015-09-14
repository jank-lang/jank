#pragma once

#include <jank/translate/environment/scope.hpp>
#include <jank/interpret/environment/scope.hpp>

namespace jank
{
  namespace interpret
  {
    namespace plugin
    {
      std::shared_ptr<environment::scope> apply
      (
        std::shared_ptr<translate::environment::scope> const &trans_scope,
        std::shared_ptr<environment::scope> const &int_scope
      );
    }
  }
}
