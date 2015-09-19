#include <jank/translate/plugin/assertion/assertion.hpp>
#include <jank/translate/plugin/detail/make_function.hpp>
#include <jank/translate/expect/error/assertion/exception.hpp>
#include <jank/interpret/environment/resolve_value.hpp>
#include <jank/interpret/expect/type.hpp>

namespace jank
{
  namespace translate
  {
    namespace plugin
    {
      namespace assertion
      {
        void assertion(std::shared_ptr<environment::scope> const &scope)
        {
          detail::make_function
          (
            scope, "assert",
            environment::builtin::type::null(*scope),
            environment::builtin::type::boolean(*scope)
          );
          detail::make_function
          (
            scope, "assert",
            environment::builtin::type::null(*scope),
            environment::builtin::type::boolean(*scope),
            environment::builtin::type::string(*scope)
          );

          detail::make_function
          (
            scope, "assert-not",
            environment::builtin::type::null(*scope),
            environment::builtin::type::boolean(*scope)
          );
          detail::make_function
          (
            scope, "assert-not",
            environment::builtin::type::null(*scope),
            environment::builtin::type::boolean(*scope),
            environment::builtin::type::string(*scope)
          );
        }
      }
    }
  }
}
