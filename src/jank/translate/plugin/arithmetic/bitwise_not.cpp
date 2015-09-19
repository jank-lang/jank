#include <jank/translate/plugin/arithmetic/bitwise_not.hpp>
#include <jank/translate/plugin/detail/make_function.hpp>

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
