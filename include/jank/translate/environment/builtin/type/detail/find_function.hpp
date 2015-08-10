#include <jank/translate/environment/scope.hpp>
#include <jank/translate/cell/detail/function_reference.hpp>

namespace jank
{
  namespace translate
  {
    namespace environment
    {
      namespace builtin
      {
        namespace type
        {
          namespace detail
          {
            inline cell::detail::function_reference
            <cell::detail::function_definition<cell::cell>>
            find_function
            (scope &s, std::string const &name)
            { return { s.find_function(name).value().at(0).first.data }; }

            inline cell::detail::function_reference
            <cell::detail::native_function_definition<cell::cell>>
            find_native_function
            (scope &s, std::string const &name)
            { return { s.find_native_function(name).value().at(0).first.data }; }
          }
        }
      }
    }
  }
}
