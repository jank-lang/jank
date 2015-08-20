#include <jank/interpret/interpret.hpp>
#include <jank/interpret/detail/if_statement.hpp>
#include <jank/interpret/environment/resolve_value.hpp>
#include <jank/interpret/expect/type.hpp>

namespace jank
{
  namespace interpret
  {
    namespace detail
    {
      cell::cell if_statement
      (
        std::shared_ptr<environment::scope> const &scope,
        translate::cell::if_statement const &cell
      )
      {
        auto const condition
        (environment::resolve_value(scope, cell.data.condition));
        if(expect::type<cell::type::boolean>(condition).data)
        { return interpret(scope, { cell.data.true_body }); }
        else
        { return interpret(scope, { cell.data.false_body }); }
      }
    }
  }
}
