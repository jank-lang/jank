#include <jank/parse/expect/type.hpp>
#include <jank/translate/plugin/compare/equal.hpp>
#include <jank/translate/plugin/compare/detail/make_operator.hpp>
#include <jank/interpret/environment/resolve_value.hpp>

namespace jank
{
  namespace translate
  {
    namespace plugin
    {
      namespace compare
      {
        void equal(std::shared_ptr<environment::scope> const &scope)
        {
          detail::make_operator
          (
            scope, "==", environment::builtin::type::boolean(*scope),
            [](auto const &scope, auto const &args)
            {
              return environment::builtin::value::boolean
              (
                parse::expect::type<parse::cell::type::boolean>
                (interpret::resolve_value(scope, args[0].cell)).data ==
                parse::expect::type<parse::cell::type::boolean>
                (interpret::resolve_value(scope, args[1].cell)).data
              );
            }
          );
          detail::make_operator
          (
            scope, "==", environment::builtin::type::integer(*scope),
            [](auto const &scope, auto const &args)
            {
              return environment::builtin::value::boolean
              (
                parse::expect::type<parse::cell::type::integer>
                (interpret::resolve_value(scope, args[0].cell)).data ==
                parse::expect::type<parse::cell::type::integer>
                (interpret::resolve_value(scope, args[1].cell)).data
              );
            }
          );
          detail::make_operator
          (
            scope, "==", environment::builtin::type::real(*scope),
            [](auto const &scope, auto const &args)
            {
              /* TODO: Use an approximate equality? */
              return environment::builtin::value::boolean
              (
                parse::expect::type<parse::cell::type::real>
                (interpret::resolve_value(scope, args[0].cell)).data ==
                parse::expect::type<parse::cell::type::real>
                (interpret::resolve_value(scope, args[1].cell)).data
              );
            }
          );
          detail::make_operator
          (
            scope, "==", environment::builtin::type::string(*scope),
            [](auto const &scope, auto const &args)
            {
              return environment::builtin::value::boolean
              (
                parse::expect::type<parse::cell::type::string>
                (interpret::resolve_value(scope, args[0].cell)).data ==
                parse::expect::type<parse::cell::type::string>
                (interpret::resolve_value(scope, args[1].cell)).data
              );
            }
          );
        }
      }
    }
  }
}
