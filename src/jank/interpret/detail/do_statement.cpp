#include <jank/interpret/interpret.hpp>
#include <jank/interpret/detail/do_statement.hpp>

namespace jank
{
  namespace interpret
  {
    namespace detail
    {
      cell::cell do_statement
      (
        std::shared_ptr<environment::scope> const &scope,
        translate::cell::do_statement const &cell
      )
      { return interpret(scope, { cell.data.body }); }
    }
  }
}
