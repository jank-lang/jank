#pragma once

#include <map>
#include <string>
#include <memory>
#include <experimental/optional>

#include <jank/translate/cell/cell.hpp>
#include <jank/interpret/function/argument.hpp>
#include <jank/interpret/expect/error/type/type.hpp>

namespace jank
{
  namespace interpret
  {
    namespace environment
    {
      struct scope
      {
        std::experimental::optional<jank::translate::cell::function_definition> find_function
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

        std::map<std::string, std::vector<translate::cell::function_definition>> function_definitions;
        std::shared_ptr<scope> parent;
      };
    }
  }
}
