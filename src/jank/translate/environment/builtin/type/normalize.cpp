#include <jank/translate/environment/builtin/type/normalize.hpp>
#include <jank/translate/environment/builtin/type/function.hpp>
#include <jank/translate/environment/builtin/type/primitive.hpp>

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
          cell::detail::type_definition<cell::cell> normalize
          (cell::detail::type_definition<cell::cell> type, scope &s)
          {
            if(type.name == "ƒ")
            { return function(s).definition; }
            if(type.name == "Ɐ")
            { return automatic(s).definition; }
            return type;
          }
        }
      }
    }
  }
}
