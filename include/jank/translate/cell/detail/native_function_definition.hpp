#pragma once

#include <jank/translate/function/argument/definition.hpp>
#include <jank/translate/function/argument/call.hpp>
#include <jank/interpret/environment/scope.hpp>

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
        template <typename C>
        struct native_function_definition
        {
          std::string name;
          function::argument::type_list arguments;
          type_reference return_type;
          std::function
          <
            C
            (
              std::shared_ptr<interpret::environment::scope> const&,
              function::argument::value_list<C> const&
            )
          > interpret;
          std::shared_ptr<environment::scope> scope;
        };
      }
    }
  }
}
