#include <jank/translate/plugin/arithmetic/subtract.hpp>
#include <jank/translate/plugin/arithmetic/detail/make_operator.hpp>

namespace jank
{
  namespace translate
  {
    namespace plugin
    {
      namespace arithmetic
      {
        void subtract(std::shared_ptr<environment::scope> const &scope)
        {
          detail::make_operator
          (scope, "-", environment::builtin::type::integer(*scope));
          detail::make_operator
          (scope, "-", environment::builtin::type::real(*scope));
        }
      }
    }
  }
}
