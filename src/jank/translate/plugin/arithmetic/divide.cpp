#include <jank/translate/plugin/arithmetic/divide.hpp>
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
        /* TODO: Handle division by zero. */
        void divide(std::shared_ptr<environment::scope> const &scope)
        {
          detail::make_operator
          (
            scope, "/", environment::builtin::type::integer(*scope),
            [](auto const &scope, auto const &args)
            {
              auto ret
              (
                interpret::expect::type<interpret::cell::type::integer>
                (interpret::environment::resolve_value(scope, args[0].cell)).data
              );
              ret /= interpret::expect::type<interpret::cell::type::integer>
              (interpret::environment::resolve_value(scope, args[1].cell)).data;
              return environment::builtin::value::integer(ret);
            }
          );
          detail::make_operator
          (
            scope, "/", environment::builtin::type::real(*scope),
            [](auto const &scope, auto const &args)
            {
              auto ret
              (
                interpret::expect::type<interpret::cell::type::real>
                (interpret::environment::resolve_value(scope, args[0].cell)).data
              );
              ret /= interpret::expect::type<interpret::cell::type::real>
              (interpret::environment::resolve_value(scope, args[1].cell)).data;
              return environment::builtin::value::real(ret);
            }
          );
        }
      }
    }
  }
}
