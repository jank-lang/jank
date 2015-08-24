#include <jank/translate/environment/scope.hpp>
#include <jank/translate/cell/detail/type_reference.hpp>

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
            inline auto make_type
            (std::string const &name)
            {
              return std::make_pair
              (
                name,
                cell::type_definition{ { name, {} } }
              );
            }

            inline auto make_type
            (
              std::string const &name,
              translate::type::generic::genericity
              <cell::type_definition::type> const &generics
            )
            {
              return std::make_pair
              (
                name,
                cell::type_definition{ { name, generics } }
              );
            }
          }
        }
      }
    }
  }
}
