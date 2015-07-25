#include <vector>
#include <memory>
#include <algorithm>

#include <jank/translate/environment/builtin/type/primitive.hpp>
#include <jank/translate/environment/special/return_statement.hpp>
#include <jank/translate/function/return/make_implicit.hpp>
#include <jank/translate/expect/type.hpp>
#include <jank/translate/expect/error/syntax/exception.hpp>

namespace jank
{
  namespace translate
  {
    namespace function
    {
      namespace ret
      {
        namespace detail
        {
          template <typename It>
          static void validate(It const begin, It const end)
          {
            // hit if statement, recurse for true and false

            // hit return statement, assert no more code is after

            // if begin + 1 == end
            //  check implicit return on begin
          }
        }

        /* TODO: once we have control logic, this will likely need to recurse. */
        void validate(cell::function_body::type &body)
        {
          auto const it
          (
            std::find_if
            (
              body.cells.begin(), body.cells.end(),
              [](auto const &c)
              { return expect::is<cell::type::return_statement>(c); }
            )
          );

          /* No return statement found. */
          if(it == body.cells.end())
          {
            auto const &null(environment::builtin::type::null(*body.scope));
            if(body.return_type == null)
            {
              /* Add an empty return. */
              body.cells.push_back
              (function::ret::make_implicit(body));
              return;
            }

            /* The previous function call may suffice as an implicit return. */
            auto const implicit(function::ret::make_implicit_from_call(body));
            if(implicit) /* Turn the last call into a return. */
            {
              body.cells.back() = implicit.value();
              return;
            }

            /* There may be an if/else with returns. */
            auto if_opt /* TODO: copied? */
            (
              expect::optional_cast<cell::type::if_statement>
              (body.cells.back())
            );
            if(if_opt)
            {
              validate(if_opt.value().data.true_body);
              validate(if_opt.value().data.false_body);
              return;
            }

            auto do_opt /* TODO: copied? */
            (
              expect::optional_cast<cell::type::do_statement>
              (body.cells.back())
            );
            if(do_opt)
            {
              validate(do_opt.value().data.body);
              return;
            }

            throw expect::error::type::exception<>
            { "not all code paths return a value" };
          }
          else /* Found at least one return statement. */
          {
            if(std::distance(it, body.cells.end()) > 1)
            { throw expect::error::syntax::exception<>{ "extraneous statements after return" }; }
          }
        }
      }
    }
  }
}
