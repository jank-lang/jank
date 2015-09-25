#include <jank/interpret/environment/scope.hpp>

namespace jank
{
  namespace interpret
  {
    namespace environment
    {
      boost::optional<cell::cell> scope::find_binding
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

      boost::optional<plugin::detail::native_function_definition>
      scope::find_native_function
      (
        translate::cell::detail::native_function_declaration
        <translate::cell::cell> const &def
      ) const
      {
        auto const it(native_function_definitions.find(def));
        if(it == native_function_definitions.end())
        {
          if(parent)
          { return parent->find_native_function(def); }
          else
          { return {}; }
        }
        return { it->second };
      }
    }
  }
}
