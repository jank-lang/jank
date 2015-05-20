#include <jank/translate/environment/scope.hpp>
#include <jank/translate/expect/error/type/type.hpp>
#include <jank/translate/expect/error/internal/internal.hpp>

namespace jank
{
  namespace translate
  {
    namespace environment
    {
      std::experimental::optional<cell::type_definition> scope::find_type
      (std::string const &name)
      {
        auto const it(type_definitions.find(name));
        if(it == type_definitions.end())
        {
          if(parent)
          { return parent->find_type(name); }
          else
          { return {}; }
        }

        return { it->second };
      }

      std::experimental::optional<cell::variable_definition> scope::find_variable
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

        return { it->second };
      }

      /* Build a vector of all function overloads in each scope, from this to the root. */
      std::experimental::optional<std::vector<cell::function_definition>> scope::find_function
      (std::string const &name)
      {
        std::vector<cell::function_definition> ret;

        /* Add all from the current scope. */
        auto const it(function_definitions.find(name));
        if(it != function_definitions.end())
        {
          std::copy
          (
            it->second.begin(), it->second.end(),
            std::back_inserter(ret)
          );
        }

        /* Add all from the parent scope(s). */
        if(parent)
        {
          auto const parent_ret(parent->find_function(name));
          if(parent_ret)
          {
            auto const &found(parent_ret.value());
            std::copy
            (
              found.begin(), found.end(),
              std::back_inserter(ret)
            );
          }
        }

        if(ret.empty())
        { return {}; }
        else
        { return { ret }; }
      }
    }
  }
}
