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
#include <jank/translate/function/argument/call.hpp>
#include <jank/translate/function/match_overload.hpp>
#include <jank/translate/function/return/validate.hpp>
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
      cell::type_reference return_type
    )
    {
      cell::function_body translated{ { {}, return_type.data, scope } };
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
          auto const &list(list_opt.value());

          /* Handle specials. */
          auto const special_opt
          (environment::special::apply_all(list, translated));
          if(special_opt)
          {
            translated.data.cells.push_back(special_opt.value());
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
          auto native_function_opt
          (scope->find_native_function(function_name));
          auto function_opt
          (scope->find_function(function_name));
          auto const function_binding
          (scope->find_binding(function_name));

          /* Allow the binding to override the functions. */
          /* TODO: Refactor this out. */
          if(function_binding)
          {
            auto const &def(function_binding.value().first);
            if
            (
              auto func_opt = expect::optional_cast
              <cell::type::function_reference>(def.data.cell)
            )
            {
              function_opt =
              { { { { func_opt.value().data.definition }, {} } } };
            }
            else if
            (
              auto native_func_opt = expect::optional_cast
              <cell::type::native_function_reference>(def.data.cell)
            )
            {
              native_function_opt =
              { { { { native_func_opt.value().data.definition }, {} } } };
            }
          }

          /* Try to match native and non-native overloads. */
          auto matched
          (
            function::match_overload
            (
              list, scope, native_function_opt, function_opt,
              [&](auto const &match)
              { translated.data.cells.push_back(match); }
            )
          );
          if(!matched)
          {
            throw expect::error::lookup::exception<>
            {
              "invalid function: " +
              parse::expect::type<parse::cell::type::ident>(list.data[0]).data
            };
          }
          else
          { continue; }
        }

        /* Treat plain values as an implicit return. */
        translated.data.cells.push_back
        (
          environment::special::return_statement
          (
            parse::cell::list
            {
              {
                parse::cell::ident{ "return" },
                c
              }
            },
            translated
          )
        );
      }

      return translated;
    }
  }
}
