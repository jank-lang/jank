#include <jank/translate/environment/builtin/type/primitive.hpp>
#include <jank/translate/environment/builtin/type/list.hpp>
#include <jank/interpret/plugin/detail/make_operator.hpp>
#include <jank/interpret/plugin/collection/list/first.hpp>

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
          void first
          (
            std::shared_ptr<translate::environment::scope> const &trans_scope,
            std::shared_ptr<environment::scope> const &int_scope
          )
          {
            detail::make_operator
            (
              trans_scope,
              int_scope,
              "first",
              translate::environment::builtin::type::list
              (
                *trans_scope,
                translate::environment::builtin::type::integer(*trans_scope)
              ),
              detail::unary_operator_wrapper
              <
                cell::type::list,
                cell::type::integer
              >{},
              [](auto const &list)
              {
                if(list.empty())
                {
                  /* TODO: Use a more suitable exception. */
                  throw expect::error::type::exception<>
                  { "no first in list" };
                }

                return expect::type<interpret::cell::type::integer>
                (list.front()).data;
              }
            );
          }
        }
      }
    }
  }
}
