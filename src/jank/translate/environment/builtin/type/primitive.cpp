#include <jank/translate/environment/builtin/type/primitive.hpp>
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
          scope& add_primitives
          (scope &s)
          {
            s.type_definitions.insert(detail::make_type("auto"));
            s.type_definitions.insert(detail::make_type("Ɐ"));
            s.type_definitions.insert(detail::make_type("null"));
            s.type_definitions.insert(detail::make_type("boolean"));
            s.type_definitions.insert(detail::make_type("integer"));
            s.type_definitions.insert(detail::make_type("real"));
            s.type_definitions.insert(detail::make_type("string"));
            return s;
          }

          cell::detail::type_reference<cell::cell> automatic(scope &s)
          { return detail::find_type(s, "auto"); }
          cell::detail::type_reference<cell::cell> null(scope &s)
          { return detail::find_type(s, "null"); }
          cell::detail::type_reference<cell::cell> boolean(scope &s)
          { return detail::find_type(s, "boolean"); }
          cell::detail::type_reference<cell::cell> integer(scope &s)
          { return detail::find_type(s, "integer"); }
          cell::detail::type_reference<cell::cell> real(scope &s)
          { return detail::find_type(s, "real"); }
          cell::detail::type_reference<cell::cell> string(scope &s)
          { return detail::find_type(s, "string"); }
        }
      }
    }
  }
}
