#pragma once

#include <jank/translate/environment/scope.hpp>
#include <jank/translate/plugin/io/print.hpp>

namespace jank
{
  namespace translate
  {
    namespace plugin
    {
      inline auto apply(std::shared_ptr<environment::scope> const &scope)
      {
        std::vector
        <
          std::function<void (std::shared_ptr<environment::scope> const&)>
        > const plugins
        {
          &io::print
        };

        for(auto const &plugin : plugins)
        { plugin(scope); }

        return scope;
      }
    }
  }
}
