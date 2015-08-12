#pragma once

#include <jank/translate/cell/detail/binding_definition.hpp>
#include <jank/translate/function/argument/call.hpp>

namespace jank
{
  namespace translate
  {
    namespace cell
    {
      namespace detail
      {
        template <typename C>
        struct indirect_function_call
        {
          binding_definition<C> binding;
          function::argument::value_list<C> arguments;
        };
      }
    }
  }
}
