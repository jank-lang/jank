#pragma once

#include <jank/translate/function/argument/definition.hpp>
#include <jank/translate/function/argument/call.hpp>

namespace jank
{
  namespace interpret
  {
    namespace environment
    { struct scope; }
  }

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
          function::argument::type_list<C> arguments;
          type_reference<C> return_type;
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
