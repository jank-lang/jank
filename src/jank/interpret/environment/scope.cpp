#include <jank/interpret/environment/scope.hpp>

namespace jank
{
  namespace interpret
  {
    namespace environment
    {
      std::experimental::optional<parse::cell::cell> scope::find_binding
      (std::string const &name) const
      {
        auto const it(bindings.find(name));
        if(it == bindings.end())
        {
          if(parent)
          { return parent->find_binding(name); }
          else
          { return {}; }
        }
        return { it->second };
      }
    }
  }
}
