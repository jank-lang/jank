#pragma once

#include <algorithm>

#include <jank/parse/cell/cell.hpp>
#include <jank/parse/cell/stream.hpp>
#include <jank/translate/cell/cell.hpp>
#include <jank/translate/cell/stream.hpp>
#include <jank/translate/environment/scope.hpp>
#include <jank/translate/environment/special/all.hpp>
#include <jank/translate/function/argument/call.hpp>
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
          if(expect::is<parse::cell::type::list>(c))
          {
            auto const &list(expect::type<parse::cell::type::list>(c));

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
              (expect::type<parse::cell::type::ident>(list.data[0]).data)
            );
            if(function_opt)
            {
              /* TODO: Arguments could be lists which need to be evaluated. */
              auto const arguments(function::argument::call::parse(list, scope));
              for(auto const & arg : arguments)
              {
                std::cout << "arg: " << arg.name << " " << arg.cell << std::endl;
              }
              /* TODO: Handle function calls. */
            }
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
