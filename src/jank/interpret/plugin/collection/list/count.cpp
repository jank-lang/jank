#include <jank/translate/environment/builtin/type/primitive.hpp>
#include <jank/translate/environment/builtin/type/list.hpp>
#include <jank/interpret/plugin/detail/make_operator.hpp>
#include <jank/interpret/plugin/collection/list/count.hpp>

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
          void count
          (
            std::shared_ptr<translate::environment::scope> const &trans_scope,
            std::shared_ptr<environment::scope> const &int_scope
          )
          {
            detail::make_operator
            (
              trans_scope,
              int_scope,
              "count",
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
              { return static_cast<cell::integer::type>(list.size()); }
            );
          }
        }
      }
    }
  }
}
