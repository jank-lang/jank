#include <jtl/iterator/range.hpp>

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
        cell::cell if_statement
        (
          parse::cell::list const &input,
          cell::function_body const &outer_body
        )
        {
          static std::size_t constexpr forms_required{ 3 };
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
            data.begin(), std::next(data.begin(), forms_required - 1)
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

          auto const true_range_start
          (std::next(data.begin(), forms_required - 1));
          auto const true_body
          (
            translate
            (
              jtl::it::make_range
              (true_range_start, std::next(true_range_start)),
              nested_scope,
              { outer_body.data.return_type }
            ).data
          );

          cell::function_body::type false_body
          {
            {}, outer_body.data.return_type, outer_body.data.scope
          };

          /* Add a false body, if it's there. */
          if(data.size() == forms_required + 1)
          {
            auto const false_range_start(std::next(true_range_start));
            false_body = translate
            (
              jtl::it::make_range
              (false_range_start, std::next(false_range_start)),
              nested_scope,
              { outer_body.data.return_type }
            ).data;
          }

          return
          { cell::if_statement{ { condition.cell, true_body, false_body } } };
        }
      }
    }
  }
}
