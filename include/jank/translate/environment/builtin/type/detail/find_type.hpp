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
            inline cell::detail::type_reference find_type
            (scope &s, std::string const &name)
            { return { s.find_type(name).value().first.data }; }
          }
        }
      }
    }
  }
}
