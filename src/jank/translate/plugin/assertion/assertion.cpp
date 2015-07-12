#include <jank/parse/expect/type.hpp>
#include <jank/translate/plugin/assertion/assertion.hpp>
#include <jank/translate/plugin/detail/make_function.hpp>
#include <jank/translate/expect/error/assertion/exception.hpp>
#include <jank/interpret/environment/resolve_value.hpp>

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
            [](auto const &scope, auto const &args)
            {
              auto const val
              (
                parse::expect::type<parse::cell::type::boolean>
                (interpret::resolve_value(scope, args[0].cell)).data
              );
              if(!val)
              { throw expect::error::assertion::exception<>{}; }
              return environment::builtin::value::null();
            },
            environment::builtin::type::null(*scope),
            environment::builtin::type::boolean(*scope)
          );
        }
      }
    }
  }
}
