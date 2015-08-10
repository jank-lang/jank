#include <jank/translate/environment/builtin/type/function.hpp>
#include <jank/translate/environment/builtin/type/detail/make_type.hpp>
#include <jank/translate/environment/builtin/type/detail/find_function.hpp>

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
          scope& add_function
          (scope &s)
          {
            /* TODO: Generics. */
            s.type_definitions.insert(detail::make_type("function"));
            s.type_definitions.insert(detail::make_type("ƒ"));
            return s;
          }

          cell::detail::function_reference<cell::detail::function_definition<cell::cell>>
          function(scope &s)
          { return detail::find_function(s, "function"); }
        }
      }
    }
  }
}
