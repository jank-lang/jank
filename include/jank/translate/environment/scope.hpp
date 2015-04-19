#pragma once

#include <map>
#include <string>
#include <memory>
#include <experimental/optional>

#include <jank/translate/cell/cell.hpp>
#include <jank/translate/expect/type.hpp>

namespace jank
{
  namespace translate
  {
    namespace environment
    {
      struct scope
      {
        scope() = default;
        explicit scope(std::shared_ptr<scope> const &p)
          : parent{ p }
        { }

        std::experimental::optional<cell::function_definition> find_function
        (std::string const &name)
        {
          auto const it(function_definitions.find(name));
          if(it == function_definitions.end())
          {
            if(parent)
            { return parent->find_function(name); }
            else
            { return {}; }
          }

          if(it->second.empty())
          { throw expect::error::type::type<>{ "unknown function: " + name }; }

          return { it->second[0] };
        }

        std::map<std::string, std::vector<cell::function_definition>> function_definitions;
        std::shared_ptr<scope> parent{ std::make_shared<scope>() };
      };
    }
  }
}
