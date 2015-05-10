#include <stdexcept>
#include <memory>

#include <jank/translate/environment/special/func.hpp>

#include <jank/translate/translate.hpp>
#include <jank/translate/function/argument/definition.hpp>
#include <jank/translate/environment/scope.hpp>
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
        cell::cell func
        (parse::cell::list const &input, cell::function_body const &outer_body)
        {
          auto &data(input.data);
          if(data.size() < 4)
          { throw expect::error::syntax::syntax<>{ "invalid function definition" }; }

          auto const name(expect::type<parse::cell::type::ident>(data[1]));
          auto const args(expect::type<parse::cell::type::list>(data[2]));
          auto const ret(expect::type<parse::cell::type::list>(data[3]));
          auto const nested_scope(std::make_shared<scope>(outer_body.data.scope));

          /* TODO: Add args to scope. */

          return
          {
            cell::function_definition
            {
              {
                name.data,
                function::argument::definition::parse_types(args),
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
