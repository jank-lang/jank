#include <jank/interpret/plugin/apply.hpp>
#include <jank/interpret/plugin/io/print.hpp>
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
          &assertion::assertion
        };

        for(auto const &plugin : plugins)
        { plugin(trans_scope, int_scope); }

        return int_scope;
      }
    }
  }
}
