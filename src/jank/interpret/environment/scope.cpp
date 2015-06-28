#include <jank/interpret/environment/scope.hpp>

namespace jank
{
  namespace interpret
  {
    namespace environment
    {
      std::experimental::optional<parse::cell::cell> scope::find_variable
      (std::string const &name) const
      {
        auto const it(variables.find(name));
        if(it == variables.end())
        {
          if(parent)
          { return parent->find_variable(name); }
          else
          { return {}; }
        }
        return { it->second };
      }
    }
  }
}
