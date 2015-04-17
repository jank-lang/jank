#pragma once

#include <stdexcept>

#include <jank/parse/cell/cell.hpp>
#include <jank/translate/cell/cell.hpp>
#include <jank/translate/function/argument.hpp>
#include <jank/translate/expect/type.hpp>
#include <jank/translate/expect/error/syntax/syntax.hpp>

namespace jank
{
  namespace translate
  {
    namespace environment
    {
      namespace special
      {
        inline cell::cell func
        (parse::cell::list const &input, cell::function_body const&)
        {
          auto &data(input.data);
          if(data.size() < 4)
          { throw expect::error::syntax::syntax<>{ "invalid function definition" }; }

          auto const name(expect::type<parse::cell::type::ident>(data[1]));
          auto const args(expect::type<parse::cell::type::list>(data[2]));
          auto const ret(expect::type<parse::cell::type::list>(data[3]));

          return
          {
            cell::function_definition
            {
              {
                name.data,
                function::argument::parse(args),
                {{}}
              }
            }
          };
        }
      }
    }
  }
}
