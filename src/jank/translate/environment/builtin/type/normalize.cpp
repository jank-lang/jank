#include <jank/translate/environment/builtin/type/normalize.hpp>
#include <jank/translate/environment/builtin/type/function.hpp>

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
          cell::detail::type_definition normalize
          (cell::detail::type_definition type, scope &s)
          {
            if(type.name == "ƒ")
            { return function(s).definition; }
            return type;
          }
        }
      }
    }
  }
}
