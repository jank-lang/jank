#pragma once

#include <algorithm>

#include <jank/parse/cell/cell.hpp>
#include <jank/parse/cell/stream.hpp>
#include <jank/parse/expect/type.hpp>
#include <jank/translate/cell/cell.hpp>
#include <jank/translate/cell/stream.hpp>
#include <jank/translate/cell/trait.hpp>
#include <jank/translate/environment/scope.hpp>
#include <jank/translate/environment/special/apply_all.hpp>
#include <jank/translate/environment/builtin/type/function.hpp>
#include <jank/translate/function/argument/call.hpp>
#include <jank/translate/function/match_overload.hpp>
#include <jank/translate/function/match_indirect.hpp>
#include <jank/translate/function/return/add_implicit_returns.hpp>
#include <jank/translate/expect/type.hpp>
#include <jank/translate/expect/error/syntax/exception.hpp>
#include <jank/translate/expect/error/internal/unimplemented.hpp>
#include <jank/translate/expect/error/lookup/exception.hpp>

namespace jank
{
  namespace translate
  {
    template <typename Range>
    cell::function_body translate
    (
      Range const &range,
      std::shared_ptr<environment::scope> const &scope,
      cell::function_body translated
    )
    {
      for(auto const &c : range)
      {
        if(parse::expect::is<parse::cell::type::comment>(c))
        { /* ignore */ continue; }
        else if
        (
          auto const list_opt = parse::expect::optional_cast
          <parse::cell::type::list>(c)
        )
        {
          auto const &list(*list_opt);

          /* Handle specials. */
          auto const special_opt
          (environment::special::apply_all(list, translated));
          if(special_opt)
          {
            translated.data.cells.push_back(*special_opt);
            continue;
          }

          /* Arbitrary empty lists are no good. */
          if(list.data.empty())
          {
            throw expect::error::syntax::exception<>
            { "invalid empty translation list" };
          }

          auto const &function_name
          (parse::expect::type<parse::cell::type::ident>(list.data[0]).data);
          auto const function_binding
          (scope->find_binding(function_name));

          /* Allow the binding to override the functions. */
          if(function_binding)
          {
            auto const &def(function_binding->first);
            auto const type(def.data.type);
            if
            (
              type.definition.name ==
              environment::builtin::type::function(*scope).definition.name
            )
            {
              function::match_indirect
              (
                def, list, scope,
                [&](auto const &call)
                { translated.data.cells.push_back(call); }
              );
              continue;
            }

            /* A binding has that name, but it's not a function.
             * Fall through and see if a function has the same name. */
          }

          auto macro_opt
          (scope->find_macro(function_name));
          auto native_function_opt
          (scope->find_native_function(function_name));
          auto function_opt
          (scope->find_function(function_name));

          /* Try to match native and non-native overloads. */
          function::match_overload
          (
            list, scope, macro_opt, native_function_opt, function_opt,
            [&](auto const &match)
            {
              /* TODO: Handle macros. */
              translated.data.cells.push_back(match);
            }
          );
          continue;
        }

        /* Treat plain values as an implicit return. */
        translated.data.cells.push_back
        (
          environment::special::return_statement
          (
            parse::cell::list
            { { parse::cell::ident{ "return" }, c } },
            translated
          )
        );
      }

      return translated;
    }

    template <typename Range>
    cell::function_body translate
    (
      Range const &range,
      std::shared_ptr<environment::scope> const &scope,
      cell::type_reference return_type
    )
    {
      return translate
      (
        range, scope,
        { { {}, return_type.data, scope } }
      );
    }
  }
}
