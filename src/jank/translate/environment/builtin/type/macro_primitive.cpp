#include <jank/translate/environment/builtin/type/macro_primitive.hpp>
#include <jank/translate/environment/builtin/type/detail/make_type.hpp>
#include <jank/translate/environment/builtin/type/detail/find_type.hpp>

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
          scope& add_macro_primitives
          (scope &s)
          {
            s.type_definitions.insert(detail::make_type("^atom"));
            s.type_definitions.insert(detail::make_type("^list"));
            return s;
          }

          cell::detail::type_reference<cell::cell> macro_atom(scope &s)
          { return detail::find_type(s, "^atom"); }
          cell::detail::type_reference<cell::cell> macro_list(scope &s)
          { return detail::find_type(s, "^list"); }
        }
      }
    }
  }
}
