#include <jank/translate/environment/scope.hpp>

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
            static auto make_primitive
            (std::string const &name)
            { return std::make_pair(name, cell::type_definition{ { name, {} } }); }
          }

          std::shared_ptr<scope> add_primitives
          (std::shared_ptr<scope> const &s)
          {
            s->type_definitions.insert(detail::make_primitive("null"));
            s->type_definitions.insert(detail::make_primitive("boolean"));
            s->type_definitions.insert(detail::make_primitive("integer"));
            s->type_definitions.insert(detail::make_primitive("real"));
            s->type_definitions.insert(detail::make_primitive("string"));
            return s;
          }

          namespace detail
          {
            static cell::detail::type_reference find_primitive
            (std::shared_ptr<scope> const &s, std::string const &name)
            { return { s->find_type(name).value().first.data }; }
          }

          cell::detail::type_reference null(std::shared_ptr<scope> const &s)
          { return detail::find_primitive(s, "null"); }
          cell::detail::type_reference boolean(std::shared_ptr<scope> const &s)
          { return detail::find_primitive(s, "boolean"); }
          cell::detail::type_reference integer(std::shared_ptr<scope> const &s)
          { return detail::find_primitive(s, "integer"); }
          cell::detail::type_reference real(std::shared_ptr<scope> const &s)
          { return detail::find_primitive(s, "real"); }
          cell::detail::type_reference string(std::shared_ptr<scope> const &s)
          { return detail::find_primitive(s, "string"); }
        }
      }
    }
  }
}
