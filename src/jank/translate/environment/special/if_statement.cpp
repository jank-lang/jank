#include <jank/translate/translate.hpp>
#include <jank/translate/environment/special/if_statement.hpp>
#include <jank/translate/environment/scope.hpp>
#include <jank/translate/environment/builtin/type/primitive.hpp>
#include <jank/translate/function/argument/resolve_type.hpp>
#include <jank/translate/expect/error/syntax/exception.hpp>

namespace jank
{
  namespace translate
  {
    namespace environment
    {
      namespace special
      {
        /* TODO: Handle return checking during validation. */
        cell::cell if_statement
        (parse::cell::list const &input, cell::function_body const &outer_body)
        {
          static std::size_t constexpr forms_required{ 2 };
          auto const &data(input.data);

          /* Need at least one arg: the condition. */
          if(data.size() < forms_required)
          {
            throw expect::error::syntax::exception<>
            { "invalid if statement" };
          }

          /* Just parse the condition. */
          parse::cell::list list;
          list.data.insert
          (
            list.data.end(),
            data.begin(), std::next(data.begin(), forms_required)
          );
          auto const condition
          (
            function::argument::call::parse<cell::cell>
            (list, outer_body.data.scope)[0]
          );
          auto const condition_type
          (
            function::argument::resolve_type
            (condition.cell, outer_body.data.scope)
          );

          /* First condition needs to be bool. */
          if
          (
            condition_type.data !=
            environment::builtin::type::boolean(*outer_body.data.scope).definition
          )
          {
            throw expect::error::type::exception<>
            { "non-boolean if condition" };
          }

          auto const nested_scope
          (std::make_shared<scope>(outer_body.data.scope));

          auto const body
          (
            translate /* Recurse into translate for the body. */
            (
              jtl::it::make_range
              (std::next(data.begin(), forms_required), data.end()),
              nested_scope,
              { outer_body.data.return_type }
            ).data
          );

          return { cell::if_statement{ { condition.cell, body } } };
        }
      }
    }
  }
}
