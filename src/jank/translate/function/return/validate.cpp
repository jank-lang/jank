#include <vector>
#include <memory>
#include <algorithm>

#include <jank/translate/cell/cell.hpp>
#include <jank/translate/environment/builtin/type/primitive.hpp>
#include <jank/translate/environment/special/return_statement.hpp>
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
            /* TODO: Move this logic into a separate file. */
            auto const &null(environment::builtin::type::null(*body.data.scope));
            if(body.data.return_type != null)
            {
              /* The previous function call may suffice as an implicit return. */
              auto const last_function
              (
                std::find_if
                (
                  body.data.cells.rbegin(), body.data.cells.rend(),
                  [](auto const &c)
                  {
                    return expect::is<cell::type::function_call>(c) ||
                           expect::is<cell::type::native_function_call>(c);
                  }
                )
              );

              /* The function needs to be the last cell in the body. */
              if(std::next(last_function) == body.data.cells.rend())
              {
                auto const native_opt
                (
                  expect::optional_cast<cell::type::native_function_call>
                  (*last_function)
                );
                auto const non_native_opt
                (
                  expect::optional_cast<cell::type::function_call>
                  (*last_function)
                );
                auto const match
                (
                  [&](auto const &opt)
                  {
                    if
                    (
                      !opt ||
                      opt.value().data.definition.return_type !=
                      body.data.return_type
                    )
                    { return false; }

                    auto const &func(opt.value());

                    /* Change the cell to be a return. */
                    body.data.cells.back() = cell::return_statement
                    {
                      { func, { func.data.definition.return_type.definition } }
                    };
                    return true;
                  }
                );
                if(match(native_opt) || match(non_native_opt))
                { return; }
              }

              throw expect::error::type::exception<>
              { "not all code paths return a value" };
            }

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
