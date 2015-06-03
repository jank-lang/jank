#pragma once

#include <jank/translate/cell/cell.hpp>
#include <jank/translate/environment/scope.hpp>

namespace jank
{
  namespace translate
  {
    namespace function
    {
      std::experimental::optional<cell::function_call> match_overload
      (
        parse::cell::list const &list,
        std::shared_ptr<environment::scope> const &scope,
        std::vector<environment::scope::result<cell::function_definition>> const &functions
      );
    }
  }
}
