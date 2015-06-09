#include <vector>
#include <memory>
#include <algorithm>

#include <jank/translate/cell/cell.hpp>
#include <jank/translate/environment/builtin/type/primitive.hpp>
#include <jank/translate/environment/special/return_statement.hpp>
#include <jank/translate/expect/type.hpp>
#include <jank/translate/expect/error/syntax/syntax.hpp>

namespace jank
{
  namespace translate
  {
    namespace function
    {
      namespace ret
      {
        /* TODO: once we have control logic, this will likely need to recurse. */
        void validate(cell::function_body &body)
        {
          auto const it
          (
            std::find_if
            (
              body.data.cells.begin(), body.data.cells.end(),
              [](auto const &c)
              { return expect::is<cell::type::return_statement>(c); }
            )
          );

          /* No return statement found. */
          if(it == body.data.cells.end())
          {
            /* TODO: The last function call might yield the right type! */
            auto const &null(environment::builtin::type::null(body.data.scope));
            if(body.data.return_type != null)
            { throw expect::error::type::exception<>{ "no return statement in function" }; }

            /* Add an empty return. */
            body.data.cells.push_back
            (
              environment::special::return_statement
              (
                parse::cell::list{ { parse::cell::ident{ "return" } } },
                body
              )
            );
          }
          else /* Found at least one return statement. */
          {
            if(std::distance(it, body.data.cells.end()) > 1)
            { throw expect::error::syntax::exception<>{ "extraneous statements after return" }; }
          }
        }
      }
    }
  }
}
