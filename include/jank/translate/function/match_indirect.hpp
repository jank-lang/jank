#pragma once

#include <jank/parse/cell/cell.hpp>
#include <jank/translate/cell/cell.hpp>
#include <jank/translate/environment/scope.hpp>

namespace jank
{
  namespace translate
  {
    namespace function
    {
      void match_indirect
      (
        cell::type_definition::type const &type,
        parse::cell::list const &args,
        std::shared_ptr<environment::scope> const &scope,
        std::function<void (cell::indirect_function_call)> callback
      );
    }
  }
}
