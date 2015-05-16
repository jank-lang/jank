#pragma once

#include <algorithm>

#include <jank/parse/cell/cell.hpp>
#include <jank/parse/cell/stream.hpp>
#include <jank/parse/expect/type.hpp>
#include <jank/translate/cell/cell.hpp>
#include <jank/translate/cell/stream.hpp>
#include <jank/translate/cell/trait.hpp>
#include <jank/translate/environment/scope.hpp>
#include <jank/translate/environment/special/all.hpp>
#include <jank/translate/function/argument/call.hpp>
#include <jank/translate/expect/error/syntax/syntax.hpp>
#include <jank/translate/expect/error/internal/unimplemented.hpp>

#include <jank/translate/function/argument/resolve_type.hpp>
#include <jank/translate/expect/error/type/overload.hpp> /* TODO: Refactor to new file. */

namespace jank
{
  namespace translate
  {
    template <typename Range>
    cell::function_body translate
    (
      Range const &range,
      std::shared_ptr<environment::scope> const &scope
    )
    {
      if(!std::distance(range.begin(), range.end()))
      { return { { {}, {} } }; }

      cell::function_body translated{ { {}, scope } };
      std::for_each
      (
        range.begin(), range.end(),
        [&](auto const &c)
        {
          if(parse::expect::is<parse::cell::type::list>(c))
          {
            auto const &list(parse::expect::type<parse::cell::type::list>(c));

            /* Handle specials. */
            auto const special_opt
            (
              environment::special::handle(list, translated)
            );
            if(special_opt)
            {
              translated.data.cells.push_back(special_opt.value());
              return;
            }

            if(list.data.empty())
            { throw expect::error::syntax::syntax<>{ "invalid empty list" }; }

            auto const function_opt
            (
              scope->find_function
              (parse::expect::type<parse::cell::type::ident>(list.data[0]).data)
            );
            if(function_opt)
            {
              /* TODO: Arguments could be expressions which need to be evaluated. */
              auto const arguments(function::argument::call::parse<cell::cell>(list, scope));
              for(auto const &arg : arguments)
              {
                std::cout << "arg: " << arg.name
                          << " " << arg.cell
                          << std::endl;
              }

              auto const functions(function_opt.value());
              for(auto const &overload_cell : functions)
              {
                auto const &overload(overload_cell.data);

                if(overload.arguments.size() != arguments.size())
                { continue; }

                if
                (
                  std::equal
                  (
                    overload.arguments.begin(), overload.arguments.end(),
                    arguments.begin(),
                    [&](auto const &lhs, auto const &rhs)
                    {
                      return
                      (
                        lhs.type.definition ==
                        function::argument::resolve_type(rhs.cell, scope).data
                      );
                    }
                  )
                )
                {
                  /* We have a match! */
                  translated.data.cells.push_back
                  ({ cell::function_call{ overload_cell.data, arguments, scope } });
                  return;
                }
              }

              /* No matching overload found. */
              throw expect::error::type::overload
              {
                "no matching function: " +
                parse::expect::type<parse::cell::type::ident>(list.data[0]).data
              };
            }

            /* TODO: It's a list, but the function wasn't found. Throw the above exception. */
            std::cout << "no function named "
                      << parse::expect::type<parse::cell::type::ident>(list.data[0]).data
                      << " found" << std::endl;
            return;
          }

          /* TODO: Handle plain values (only useful at the end of a function?) */
          throw expect::error::internal::unimplemented{ "atoms" };
        }
      );
      return translated;
    }
  }
}
