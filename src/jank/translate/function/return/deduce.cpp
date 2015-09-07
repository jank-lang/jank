#include <vector>
#include <memory>
#include <algorithm>

#include <jank/translate/function/return/deduce.hpp>
#include <jank/translate/function/argument/resolve_type.hpp>
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
        namespace detail
        {
          /* Checks if two types are not equivalent and one of them isn't auto.
           * This can happen if adjacent bodies are returning different types. */
          template <typename T>
          bool compatible
          (T const &t1, T const &t2, environment::scope &scope)
          {
            static auto const automatic
            (environment::builtin::type::automatic(scope));
            return t1 == t2 || t1 == automatic || t2 == automatic;
          }

          [[noreturn]] static void fail()
          {
            throw expect::error::type::exception<>
            { "incompatible return types during deduction" };
          }
        }

        cell::function_body::type deduce(cell::function_body::type body)
        {
          /* Nothing to deduce if the type isn't auto. */
          auto const automatic
          (environment::builtin::type::automatic(*body.scope));
          if(body.return_type != automatic)
          { return body; }
          else if(body.cells.empty())
          {
            body.return_type = environment::builtin::type::null(*body.scope);
            return body;
          }

          /* We're answering two things:
           *  1. What's the function's return type?
           *  2. Do all bodies return compatible types?
           */
          bool all_paths_return = false;

          /* Iterate from the bottom of the function to the top. */
          std::for_each
          (
            body.cells.rbegin(), body.cells.rend(),
            [&](auto &c)
            {
              switch(cell::trait::to_enum(c))
              {
                case cell::type::if_statement:
                {
                  auto &statement(expect::type<cell::type::if_statement>(c).data);
                  statement.true_body = deduce(std::move(statement.true_body));
                  statement.false_body = deduce(std::move(statement.false_body));

                  auto const compatible_branches
                  (
                    detail::compatible
                    (
                      statement.true_body.return_type,
                      statement.false_body.return_type,
                      *body.scope
                    )
                  );

                  auto const compatible_if
                  (
                    detail::compatible
                    (
                      statement.true_body.return_type,
                      body.return_type,
                      *body.scope
                    )
                  );

                  if(!compatible_if)
                  { detail::fail(); }

                  if(!all_paths_return)
                  {
                    if(!compatible_branches)
                    { detail::fail(); }

                    body.return_type = statement.true_body.return_type;
                    all_paths_return = true;
                  }
                } break;

                case cell::type::do_statement:
                {
                  auto &statement(expect::type<cell::type::do_statement>(c).data);
                  statement.body = deduce(std::move(statement.body));

                  auto const compatible_do
                  (
                    detail::compatible
                    (
                      statement.body.return_type,
                      body.return_type,
                      *body.scope
                    )
                  );

                  if(!compatible_do)
                  { detail::fail(); }

                  if(!all_paths_return)
                  {
                    body.return_type = statement.body.return_type;
                    all_paths_return = true;
                  }
                } break;

                case cell::type::return_statement:
                {
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
                  all_paths_return = true;
                } break;

                default:
                  break;
              }
            }
          );

          return body;
        }
      }
    }
  }
}
