#pragma once

#include <jank/interpret/environment/scope.hpp>
#include <jank/interpret/plugin/io/print.hpp>

namespace jank
{
  namespace interpret
  {
    namespace plugin
    {
      /* TODO: Move to cpp */
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
