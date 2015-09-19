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
#include <jank/interpret/plugin/assertion/assertion.hpp>

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

          &assertion::assertion
        };

        for(auto const &plugin : plugins)
        { plugin(trans_scope, int_scope); }

        return int_scope;
      }
    }
  }
}
