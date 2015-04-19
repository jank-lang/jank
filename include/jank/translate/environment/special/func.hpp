#pragma once

#include <stdexcept>

#include <jank/parse/cell/cell.hpp>
#include <jank/translate/cell/cell.hpp>
#include <jank/translate/function/argument.hpp>
#include <jank/translate/expect/type.hpp>
#include <jank/translate/expect/error/syntax/syntax.hpp>
#include <jank/translate/environment/scope.hpp>

namespace jank
{
  namespace translate
  {
    template <typename Range>
    cell::function_body translate(Range const &range, std::shared_ptr<environment::scope> const &scope);

    namespace environment
    {
      namespace special
      {
        inline cell::cell func
        (parse::cell::list const &input, cell::function_body const &body)
        {
          auto &data(input.data);
          if(data.size() < 4)
          { throw expect::error::syntax::syntax<>{ "invalid function definition" }; }

          auto const name(expect::type<parse::cell::type::ident>(data[1]));
          auto const args(expect::type<parse::cell::type::list>(data[2]));
          auto const ret(expect::type<parse::cell::type::list>(data[3]));
          auto const nested_scope(std::make_shared<scope>(body.data.scope));

          return
          {
            cell::function_definition
            {
              {
                name.data,
                function::argument::parse(args),
                translate
                (
                  jtl::it::make_range(std::next(data.begin(), 4), data.end()),
                  nested_scope
                ).data,
                nested_scope
              }
            }
          };
        }
      }
    }
  }
}
