#include <jank/interpret/interpret.hpp>
#include <jank/interpret/detail/native_function_call.hpp>
#include <jank/interpret/environment/resolve_value.hpp>

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
        return environment::resolve_value
        (scope, cell.data.definition.interpret(scope, cell.data.arguments));
      }
    }
  }
}
