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
#include <jank/translate/function/match_overload.hpp>
#include <jank/translate/expect/error/syntax/syntax.hpp>
#include <jank/translate/expect/error/internal/unimplemented.hpp>

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
          if(parse::expect::is<parse::cell::type::comment>(c))
          { /* ignore */ return; }
          else if
          (
            auto const list_opt = parse::expect::optional_cast
            <parse::cell::type::list>(c)
          )
          {
            auto const &list(list_opt.value());

            /* Handle specials. */
            auto const special_opt
            (environment::special::handle(list, translated));
            if(special_opt)
            {
              translated.data.cells.push_back(special_opt.value());
              return;
            }

            /* Arbitrary empty lists are no good. */
            if(list.data.empty())
            { throw expect::error::syntax::exception<>{ "invalid empty list" }; }

            auto const function_opt
            (
              scope->find_function
              (parse::expect::type<parse::cell::type::ident>(list.data[0]).data)
            );
            if(function_opt)
            {
              auto const matched_opt(function::match_overload(list, scope, function_opt.value()));
              if(matched_opt)
              { translated.data.cells.push_back(matched_opt.value()); }
            }

            /* TODO: It's a list, but the function wasn't found. Throw an exception. */
            //std::cout << "no function named "
            //          << parse::expect::type<parse::cell::type::ident>(list.data[0]).data
            //          << " found" << std::endl;
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
