#include <jank/translate/plugin/arithmetic/bitwise_not.hpp>
/* TODO: Just include make_function in these? */
#include <jank/translate/plugin/arithmetic/detail/make_operator.hpp>
#include <jank/interpret/environment/resolve_value.hpp>
#include <jank/interpret/expect/type.hpp>

namespace jank
{
  namespace translate
  {
    namespace plugin
    {
      namespace arithmetic
      {
        void bitwise_not(std::shared_ptr<environment::scope> const &scope)
        {
          plugin::detail::make_function
          (
            scope, "~",
            environment::builtin::type::integer(*scope),
            environment::builtin::type::integer(*scope)
          );
        }
      }
    }
  }
}
