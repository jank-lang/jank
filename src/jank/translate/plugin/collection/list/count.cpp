#include <jank/translate/plugin/collection/list/count.hpp>
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
          static void make_count
          (
            std::shared_ptr<environment::scope> const &scope,
            cell::detail::type_reference<cell::cell> const &elem_type,
            cell::detail::type_reference<cell::cell> const &col_type
          )
          {
            plugin::detail::make_function
            (
              scope, "count",
              elem_type,
              col_type
            );
          }

          void count(std::shared_ptr<environment::scope> const &scope)
          {
            make_count
            (
              scope,
              environment::builtin::type::integer(*scope),
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
