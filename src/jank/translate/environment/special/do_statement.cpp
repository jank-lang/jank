#include <jank/translate/translate.hpp>
#include <jank/translate/environment/special/do_statement.hpp>
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
        cell::cell do_statement
        (
          parse::cell::list const &input,
          cell::function_body const &outer_body
        )
        {
          static std::size_t constexpr forms_required{ 2 };
          auto const &data(input.data);

          /* Need at least one arg: the condition. */
          if(data.size() < forms_required)
          {
            throw expect::error::syntax::exception<>
            { "invalid do statement" };
          }

          auto const nested_scope
          (std::make_shared<scope>(outer_body.data.scope));

          auto const body
          (
            translate
            (
              jtl::it::make_range
              (std::next(data.begin()), data.end()),
              nested_scope,
              { outer_body.data.return_type }
            ).data
          );

          return
          { cell::do_statement{ { body } } };
        }
      }
    }
  }
}
