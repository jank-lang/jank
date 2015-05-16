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
            s->type_definitions.insert(detail::make_primitive("boolean"));
            s->type_definitions.insert(detail::make_primitive("integer"));
            s->type_definitions.insert(detail::make_primitive("real"));
            s->type_definitions.insert(detail::make_primitive("string"));
            return s;
          }
        }
      }
    }
  }
}
