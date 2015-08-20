#include <jank/interpret/interpret.hpp>
#include <jank/interpret/detail/function_call.hpp>
#include <jank/interpret/environment/resolve_value.hpp>

namespace jank
{
  namespace interpret
  {
    namespace detail
    {
      cell::cell function_call
      (
        std::shared_ptr<environment::scope> const &scope,
        translate::cell::function_call const &cell
      )
      {
        auto const next_scope(std::make_shared<environment::scope>());
        next_scope->parent = scope;

        auto arg_name_it(cell.data.definition.arguments.begin());
        for(auto const &arg : cell.data.arguments)
        {
          auto const &name(*arg_name_it++);
          auto const var(environment::resolve_value(next_scope, arg.cell));
          next_scope->bindings[name.name] = var;
        }

        return interpret(next_scope, { cell.data.definition.body });
      }
    }
  }
}
