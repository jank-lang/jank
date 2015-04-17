#pragma once

#include <stdexcept>

#include <jank/parse/cell/cell.hpp>
#include <jank/translate/cell/cell.hpp>
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

          return
          {
            cell::function_definition
            {
              {
                expect::type<parse::cell::type::ident>(input.data[1]).data,
                {},
                {{}}
              }
            }
          };
        }
      }
    }
  }
}
