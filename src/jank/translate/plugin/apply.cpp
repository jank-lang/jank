#include <jank/translate/plugin/apply.hpp>
#include <jank/translate/plugin/io/print.hpp>
#include <jank/translate/plugin/arithmetic/add.hpp>

namespace jank
{
  namespace translate
  {
    namespace plugin
    {
      std::shared_ptr<environment::scope> apply
      (std::shared_ptr<environment::scope> const &scope)
      {
        /* TODO: Refactor to some shared collection of plugins; shared with interpret. */
        std::vector
        <
          std::function<void (std::shared_ptr<environment::scope> const&)>
        > const plugins
        {
          &io::print,
          &arithmetic::add
        };

        for(auto const &plugin : plugins)
        { plugin(scope); }

        return scope;
      }
    }
  }
}
