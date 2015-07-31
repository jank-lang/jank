#include <vector>
#include <memory>
#include <algorithm>

#include <jank/translate/function/return/deduce.hpp>
#include <jank/translate/function/argument/resolve_type.hpp>
#include <jank/translate/environment/builtin/type/primitive.hpp>
#include <jank/translate/environment/special/return_statement.hpp>
#include <jank/translate/expect/type.hpp>
#include <jank/translate/expect/error/syntax/exception.hpp>
#include <jank/translate/cell/stream.hpp> // TODO

namespace jank
{
  namespace translate
  {
    namespace function
    {
      namespace ret
      {
        cell::function_body::type deduce(cell::function_body::type body)
        {
          /* Nothing to deduce if the type isn't auto. */
          auto const automatic
          (environment::builtin::type::automatic(*body.scope));
          if(body.return_type != automatic)
          { return body; }

          for(auto &c : body.cells)
          {
            auto const is_return(expect::is<cell::type::return_statement>(c));
            if(!is_return)
            { continue; }

            auto &statement(expect::type<cell::type::return_statement>(c));
            auto const type
            (argument::resolve_type(statement.data.cell, body.scope));

            if(body.return_type == automatic)
            {
              statement.data.expected_type = { type.data };
              body.return_type = { type.data };
            }
            else if(body.return_type.definition != type.data)
            {
              throw expect::error::type::exception<>
              { "unable to deduce return type; mismatched types" };
            }
          }

          std::cout << cell::function_body{ body } << std::endl;

          return body;
        }
      }
    }
  }
}
