#include <jank/translate/environment/scope.hpp>
#include <jank/translate/expect/type.hpp>

namespace jank
{
  namespace translate
  {
    namespace environment
    {
      std::experimental::optional<std::vector<cell::variable_definition>> scope::find_variable
      (std::string const &name)
      {
        auto const it(variable_definitions.find(name));
        if(it == variable_definitions.end())
        {
          if(parent)
          { return parent->find_variable(name); }
          else
          { return {}; }
        }

        if(it->second.empty())
        { throw expect::error::type::type<>{ "unknown variable: " + name }; }

        return { it->second };
      }

      std::experimental::optional<std::vector<cell::function_definition>> scope::find_function
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

        return { it->second };
      }
    }
  }
}
