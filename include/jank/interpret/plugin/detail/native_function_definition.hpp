#pragma once

#include <jank/translate/function/argument/definition.hpp>
#include <jank/translate/function/argument/call.hpp>
#include <jank/interpret/cell/cell.hpp>

namespace jank
{
  namespace interpret
  {
    namespace environment
    { struct scope; }

    namespace plugin
    {
      namespace detail
      {
        struct native_function_definition
        {
          using function_type = std::function
          <
            cell::cell
            (
              std::shared_ptr<environment::scope> const&,
              translate::function::argument::value_list
              <translate::cell::cell> const&
            )
          >;

          std::string name;
          function_type interpret;
        };
      }
    }
  }
}
