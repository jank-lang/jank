#include <jank/translate/plugin/collection/list/rest.hpp>
#include <jank/translate/plugin/detail/make_function.hpp>
#include <jank/translate/environment/builtin/type/primitive.hpp>
#include <jank/translate/environment/builtin/type/list.hpp>
#include <jank/translate/environment/builtin/value/primitive.hpp>

namespace jank
{
  namespace translate
  {
    namespace plugin
    {
      namespace collection
      {
        namespace list
        {
          static void make_rest
          (
            std::shared_ptr<environment::scope> const &scope,
            cell::detail::type_reference<cell::cell> const &col_type
          )
          {
            plugin::detail::make_function
            (
              scope, "rest",
              col_type,
              col_type
            );
          }

          void rest(std::shared_ptr<environment::scope> const &scope)
          {
            make_rest
            (
              scope,
              environment::builtin::type::list
              (
                *scope,
                environment::builtin::type::integer(*scope)
              )
            );
          }
        }
      }
    }
  }
}
