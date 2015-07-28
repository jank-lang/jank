#include <jank/interpret/interpret.hpp>
#include <jank/interpret/detail/binding_definition.hpp>
#include <jank/interpret/environment/resolve_value.hpp>

namespace jank
{
  namespace interpret
  {
    namespace detail
    {
      parse::cell::cell binding_definition
      (
        std::shared_ptr<environment::scope> const &scope,
        translate::cell::binding_definition const &cell
      )
      {
        return scope->bindings[cell.data.name] = resolve_value
        (scope, cell.data.cell);
      }
    }
  }
}
