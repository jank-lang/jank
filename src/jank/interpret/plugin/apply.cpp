#include <jank/interpret/plugin/apply.hpp>
#include <jank/interpret/plugin/io/print.hpp>
#include <jank/interpret/plugin/arithmetic/add.hpp>
#include <jank/interpret/plugin/arithmetic/subtract.hpp>
#include <jank/interpret/plugin/arithmetic/multiply.hpp>
#include <jank/interpret/plugin/arithmetic/divide.hpp>
#include <jank/interpret/plugin/arithmetic/modulo.hpp>
#include <jank/interpret/plugin/arithmetic/bitwise_and.hpp>
#include <jank/interpret/plugin/arithmetic/bitwise_or.hpp>
#include <jank/interpret/plugin/arithmetic/bitwise_xor.hpp>
#include <jank/interpret/plugin/arithmetic/bitwise_not.hpp>
#include <jank/interpret/plugin/arithmetic/bitwise_left_shift.hpp>
#include <jank/interpret/plugin/arithmetic/bitwise_right_shift.hpp>
#include <jank/interpret/plugin/compare/equal.hpp>
#include <jank/interpret/plugin/compare/less.hpp>
#include <jank/interpret/plugin/compare/less_equal.hpp>
#include <jank/interpret/plugin/compare/greater.hpp>
#include <jank/interpret/plugin/compare/greater_equal.hpp>
#include <jank/interpret/plugin/assertion/assertion.hpp>
#include <jank/interpret/plugin/collection/list/cons.hpp>
#include <jank/interpret/plugin/collection/list/first.hpp>
#include <jank/interpret/plugin/collection/list/rest.hpp>
#include <jank/interpret/plugin/collection/list/count.hpp>
#include <jank/interpret/plugin/collection/list/list.hpp>

namespace jank
{
  namespace interpret
  {
    namespace plugin
    {
      std::shared_ptr<environment::scope> apply
      (
        std::shared_ptr<translate::environment::scope> const &trans_scope,
        std::shared_ptr<environment::scope> const &int_scope
      )
      {
        std::vector
        <
          std::function
          <
            void
            (
              std::shared_ptr<translate::environment::scope> const&,
              std::shared_ptr<environment::scope> const&
            )
          >
        > const plugins
        {
          &io::print,

          &arithmetic::add,
          &arithmetic::subtract,
          &arithmetic::multiply,
          &arithmetic::divide,
          &arithmetic::modulo,
          &arithmetic::bitwise_and,
          &arithmetic::bitwise_or,
          &arithmetic::bitwise_xor,
          &arithmetic::bitwise_not,
          &arithmetic::bitwise_left_shift,
          &arithmetic::bitwise_right_shift,

          &compare::equal,
          &compare::less,
          &compare::less_equal,
          &compare::greater,
          &compare::greater_equal,

          &assertion::assertion,

          &collection::list::cons,
          &collection::list::first,
          &collection::list::rest,
          &collection::list::count,
          &collection::list::list
        };

        for(auto const &plugin : plugins)
        { plugin(trans_scope, int_scope); }

        return int_scope;
      }
    }
  }
}
