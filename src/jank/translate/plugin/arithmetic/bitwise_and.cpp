#include <jank/translate/plugin/arithmetic/bitwise_and.hpp>
#include <jank/translate/plugin/arithmetic/detail/make_operator.hpp>

namespace jank
{
  namespace translate
  {
    namespace plugin
    {
      namespace arithmetic
      {
        void bitwise_and(std::shared_ptr<environment::scope> const &scope)
        {
          detail::make_operator
          (scope, "&", environment::builtin::type::integer(*scope));
        }
      }
    }
  }
}
