#include <jank/parse/expect/type.hpp>
#include <jank/interpret/interpret.hpp>
#include <jank/interpret/detail/return_statement.hpp>
#include <jank/interpret/environment/resolve_value.hpp>

namespace jank
{
  namespace interpret
  {
    namespace detail
    {
      parse::cell::cell return_statement
      (
        std::shared_ptr<environment::scope> const &scope,
        translate::cell::return_statement const &cell
      )
      { return resolve_value(scope, cell.data.cell); }
    }
  }
}
