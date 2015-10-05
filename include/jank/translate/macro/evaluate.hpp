#include <jank/translate/cell/cell.hpp>
#include <jank/translate/environment/scope.hpp>
#include <jank/interpret/interpret.hpp>
#include <jank/interpret/plugin/apply.hpp>

namespace jank
{
  namespace translate
  {
    namespace macro
    {
      /* For function calls, this does nothing. */
      template <typename T>
      T evaluate
      (
        T &&call,
        std::shared_ptr<environment::scope> const&
      )
      { return std::forward<T>(call); }

      /* For macros, this interprets in place and stores the results. */
      inline cell::macro_call evaluate
      (
        cell::macro_call &&call,
        std::shared_ptr<environment::scope> const &scope
      )
      {
        auto const interpret_scope
        (std::make_shared<interpret::environment::scope>());
        interpret::plugin::apply(scope, interpret_scope);

        /* TODO: Only allow ^emit in macros. */

        /* TODO: Native emit function which sets global state? */
        interpret::interpret
        (
          interpret_scope,
          cell::function_body{ call.data.definition.body }
        );

        return std::move(call);
      }
    }
  }
}
