#include <jank/interpret/interpret.hpp>
#include <jank/interpret/detail/indirect_function_call.hpp>
#include <jank/interpret/environment/resolve_value.hpp>
#include <jank/interpret/expect/type.hpp>

namespace jank
{
  namespace interpret
  {
    namespace detail
    {
      cell::cell indirect_function_call
      (
        std::shared_ptr<environment::scope> const &scope,
        translate::cell::indirect_function_call const &cell
      )
      {
        auto const next_scope(std::make_shared<environment::scope>());
        next_scope->parent = scope;

        auto arg_name_it(cell.data.arguments.begin());
        for(auto const &arg : cell.data.arguments)
        {
          auto const &name(*arg_name_it++);
          auto const var(environment::resolve_value(next_scope, arg.cell));
          next_scope->bindings[name.name] = var;
        }

        auto const func_cell
        (
          environment::resolve_value
          (scope, translate::cell::binding_reference{ { cell.data.binding } })
        );
        auto const func(expect::type<cell::type::function>(func_cell));
        return interpret(next_scope, { func.data.body });
      }
    }
  }
}
