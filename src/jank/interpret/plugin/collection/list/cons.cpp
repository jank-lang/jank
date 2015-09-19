#include <jank/translate/environment/builtin/type/primitive.hpp>
#include <jank/translate/environment/builtin/type/list.hpp>
#include <jank/interpret/plugin/detail/make_operator.hpp>
#include <jank/interpret/plugin/collection/list/cons.hpp>

namespace jank
{
  namespace interpret
  {
    namespace plugin
    {
      namespace collection
      {
        namespace list
        {
          void cons
          (
            std::shared_ptr<translate::environment::scope> const &trans_scope,
            std::shared_ptr<environment::scope> const &int_scope
          )
          {
            detail::make_operator
            (
              trans_scope,
              int_scope,
              "cons",
              translate::environment::builtin::type::integer(*trans_scope),
              translate::environment::builtin::type::list
              (
                *trans_scope,
                translate::environment::builtin::type::integer(*trans_scope)
              ),
              detail::binary_operator_wrapper
              <
                cell::type::integer,
                cell::type::list,
                cell::type::list
              >{},
              [](auto const &elem, auto list)
              {
                list.push_front(interpret::cell::integer{ elem });
                return list;
              }
            );
          }
        }
      }
    }
  }
}
