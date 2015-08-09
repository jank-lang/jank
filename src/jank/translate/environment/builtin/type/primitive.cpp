#include <jank/translate/environment/builtin/type/primitive.hpp>
#include <jank/translate/environment/builtin/type/detail/make_type.hpp>

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
            s.type_definitions.insert(detail::make_type("null"));
            s.type_definitions.insert(detail::make_type("boolean"));
            s.type_definitions.insert(detail::make_type("integer"));
            s.type_definitions.insert(detail::make_type("real"));
            s.type_definitions.insert(detail::make_type("string"));
            return s;
          }

          namespace detail
          {
            static cell::detail::type_reference find_primitive
            (scope &s, std::string const &name)
            { return { s.find_type(name).value().first.data }; }
          }

          cell::detail::type_reference automatic(scope &s)
          { return detail::find_primitive(s, "auto"); }
          cell::detail::type_reference null(scope &s)
          { return detail::find_primitive(s, "null"); }
          cell::detail::type_reference boolean(scope &s)
          { return detail::find_primitive(s, "boolean"); }
          cell::detail::type_reference integer(scope &s)
          { return detail::find_primitive(s, "integer"); }
          cell::detail::type_reference real(scope &s)
          { return detail::find_primitive(s, "real"); }
          cell::detail::type_reference string(scope &s)
          { return detail::find_primitive(s, "string"); }
        }
      }
    }
  }
}
