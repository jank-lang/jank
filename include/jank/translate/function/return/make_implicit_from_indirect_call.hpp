#pragma once

#include <jank/translate/function/argument/resolve_type.hpp>
#include <jank/translate/environment/special/return_statement.hpp>
#include <jank/translate/environment/builtin/type/primitive.hpp>
#include <jank/translate/expect/type.hpp>

namespace jank
{
  namespace translate
  {
    namespace function
    {
      namespace ret
      {
        std::experimental::optional<cell::cell>
        make_implicit_from_indirect_call
        (
          cell::indirect_function_call const &call,
          cell::function_body::type const &body
        )
        {
          auto const &type(argument::resolve_type(call, body.scope));
          if
          (
            type.data != body.return_type.definition &&
            body.return_type !=
            environment::builtin::type::automatic(*body.scope)
          )
          { return {}; }

          /* Change the cell to be a return. */
          return
          {
            cell::return_statement
            { { call, { type.data } } }
          };
        }
      }
    }
  }
}
