#include <jank/interpret/interpret.hpp>
#include <jank/interpret/detail/native_function_call.hpp>
#include <jank/interpret/environment/resolve_value.hpp>
#include <jank/interpret/expect/error/lookup/exception.hpp>

namespace jank
{
  namespace interpret
  {
    namespace detail
    {
      cell::cell native_function_call
      (
        std::shared_ptr<environment::scope> const &scope,
        translate::cell::native_function_call const &cell
      )
      {

        auto const &declaration(cell.data.definition);
        auto const &definition(scope->find_native_function(declaration));
        if(!definition)
        {
          throw expect::error::lookup::exception<>
          { "invalid native function: " + declaration.name };
        }

        return definition.value().interpret(scope, cell.data.arguments);
      }
    }
  }
}
