#include <jank/translate/environment/builtin/type/primitive.hpp>
#include <jank/interpret/plugin/detail/make_operator.hpp>
#include <jank/interpret/plugin/arithmetic/add.hpp>

namespace jank
{
  namespace interpret
  {
    namespace plugin
    {
      namespace arithmetic
      {
        void bitwise_not
        (
          std::shared_ptr<translate::environment::scope> const &trans_scope,
          std::shared_ptr<environment::scope> const &int_scope
        )
        {
          detail::make_operator
          (
            trans_scope,
            int_scope,
            "~",
            translate::environment::builtin::type::integer(*trans_scope),
            detail::unary_operator_wrapper
            <
              cell::type::integer,
              cell::type::integer
            >{},
            [](auto const &l)
            { return ~l; }
          );
        }
      }
    }
  }
}
