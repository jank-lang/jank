#include <jank/translate/plugin/apply.hpp>
#include <jank/translate/plugin/io/print.hpp>
#include <jank/translate/plugin/arithmetic/add.hpp>
#include <jank/translate/plugin/arithmetic/subtract.hpp>
#include <jank/translate/plugin/arithmetic/multiply.hpp>
#include <jank/translate/plugin/arithmetic/divide.hpp>
#include <jank/translate/plugin/compare/equal.hpp>
#include <jank/translate/plugin/compare/less.hpp>
#include <jank/translate/plugin/compare/less_equal.hpp>
#include <jank/translate/plugin/compare/greater.hpp>
#include <jank/translate/plugin/compare/greater_equal.hpp>
#include <jank/translate/plugin/assertion/assertion.hpp>

namespace jank
{
  /* TODO: Native function definitions and all of this plugin shit
   * should be in interpret. */
  namespace translate
  {
    namespace plugin
    {
      std::shared_ptr<environment::scope> apply
      (std::shared_ptr<environment::scope> const &scope)
      {
        /* TODO: Read from shared objects. */
        std::vector
        <
          std::function<void (std::shared_ptr<environment::scope> const&)>
        > const plugins
        {
          &io::print,

          &arithmetic::add,
          &arithmetic::subtract,
          &arithmetic::multiply,
          &arithmetic::divide,
          &compare::equal,
          &compare::less,
          &compare::less_equal,
          &compare::greater,
          &compare::greater_equal,
          &assertion::assertion,
        };

        for(auto const &plugin : plugins)
        { plugin(scope); }

        return scope;
      }
    }
  }
}
