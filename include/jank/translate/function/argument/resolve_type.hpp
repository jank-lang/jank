#pragma once

#include <jank/translate/cell/cell.hpp>
#include <jank/translate/environment/scope.hpp>

namespace jank
{
  namespace translate
  {
    namespace function
    {
      namespace argument
      {
        /* Arguments are stored in translation cells, but not tied directly to
         * a type_definition. This is because they can be literal values, binding
         * references, or maybe other things. */
        cell::type_definition resolve_type
        (
          cell::cell const &c,
          std::shared_ptr<environment::scope> const &scope
        );
      }
    }
  }
}
