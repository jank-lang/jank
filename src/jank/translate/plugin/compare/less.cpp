#include <jank/translate/plugin/compare/less.hpp>
#include <jank/translate/plugin/compare/detail/make_operator.hpp>

namespace jank
{
  namespace translate
  {
    namespace plugin
    {
      namespace compare
      {
        void less(std::shared_ptr<environment::scope> const &scope)
        {
          detail::make_operator
          (scope, "<", environment::builtin::type::integer(*scope));
          detail::make_operator
          (scope, "<", environment::builtin::type::real(*scope));
          detail::make_operator
          (scope, "<", environment::builtin::type::string(*scope));
        }
      }
    }
  }
}
