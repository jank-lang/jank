#include <jank/interpret/interpret.hpp>
#include <jank/interpret/detail/variable_definition.hpp>
#include <jank/interpret/environment/resolve_value.hpp>

namespace jank
{
  namespace interpret
  {
    namespace detail
    {
      parse::cell::cell variable_definition
      (
        std::shared_ptr<environment::scope> const &scope,
        translate::cell::variable_definition const &cell
      )
      {
        return scope->variables[cell.data.name] = resolve_value
        (scope, cell.data.cell);
      }
    }
  }
}
