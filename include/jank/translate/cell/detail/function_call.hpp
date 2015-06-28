#pragma once

#include <memory>
#include <vector>

#include <jank/translate/cell/detail/function_definition.hpp>
#include <jank/translate/function/argument/call.hpp>

namespace jank
{
  namespace translate
  {
    namespace environment
    { struct scope; }

    namespace cell
    {
      namespace detail
      {
        template <typename C, typename Def = function_definition<C>>
        struct function_call
        {
          Def definition;
          jank::translate::function::argument::value_list<C> arguments;
          std::shared_ptr<environment::scope> scope;
        };
      }
    }
  }
}
