#include <jank/translate/environment/builtin/type/list.hpp>
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
          scope& add_list
          (scope &s)
          {
            s.type_definitions.insert
            (
              detail::make_type
              (
                "list",
                { {
                  translate::type::generic::single<cell::type_definition::type>
                  {},
                } }
              )
            );
            return s;
          }

          cell::detail::type_reference<cell::cell> list(scope &s)
          { return detail::find_type(s, "list"); }
        }
      }
    }
  }
}
