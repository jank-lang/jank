#include <jank/translate/environment/builtin/type/function.hpp>
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
          scope& add_function
          (scope &s)
          {
            s.type_definitions.insert
            (
              detail::make_type
              (
                "function",
                { {
                  translate::type::generic::tuple<cell::type_definition::type>
                  {},
                  translate::type::generic::tuple<cell::type_definition::type>
                  {},
                } }
              )
            );
            s.type_definitions.insert // TODO
            (
              detail::make_type
              (
                "curry",
                { {
                  translate::type::generic::single<cell::type_definition::type>
                  {},
                } }
              )
            );
            return s;
          }

          cell::detail::type_reference function(scope &s)
          { return detail::find_type(s, "function"); }
        }
      }
    }
  }
}
