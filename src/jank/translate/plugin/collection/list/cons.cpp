#include <jank/interpret/cell/stream.hpp>
#include <jank/translate/plugin/collection/list/cons.hpp>
#include <jank/translate/plugin/detail/make_function.hpp>
#include <jank/translate/environment/builtin/type/primitive.hpp>
#include <jank/translate/environment/builtin/type/list.hpp>
#include <jank/translate/environment/builtin/value/primitive.hpp>
#include <jank/interpret/environment/resolve_value.hpp>
#include <jank/interpret/expect/type.hpp>

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
          static void make_cons
          (
            std::shared_ptr<environment::scope> const &scope,
            cell::detail::type_reference<cell::cell> const &elem_type,
            cell::detail::type_reference<cell::cell> const &col_type
          )
          {
            plugin::detail::make_function
            (scope, "cons", col_type, elem_type, col_type);

            ///* TODO: Remove */
            //plugin::detail::make_function
            //(
            //  scope, "cons",
            //  [](auto const &, auto const &)
            //  {
            //    std::list<parse::cell::integer> ret_coll;
            //    return cell::cell{ cell::literal_value{ ret_coll } };
            //  },
            //  col_type
            //);
          }

          void cons(std::shared_ptr<environment::scope> const &scope)
          {
            make_cons
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
