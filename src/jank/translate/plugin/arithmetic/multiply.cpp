#include <jank/translate/plugin/arithmetic/multiply.hpp>
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
        void multiply(std::shared_ptr<environment::scope> const &scope)
        {
          detail::make_operator
          (
            scope, "*", environment::builtin::type::integer(*scope),
            [](auto const &scope, auto const &args)
            {
              auto ret
              (
                interpret::expect::type<interpret::cell::type::integer>
                (interpret::environment::resolve_value(scope, args[0].cell)).data
              );
              ret *= interpret::expect::type<interpret::cell::type::integer>
              (interpret::environment::resolve_value(scope, args[1].cell)).data;
              return environment::builtin::value::integer(ret);
            }
          );
          detail::make_operator
          (
            scope, "*", environment::builtin::type::real(*scope),
            [](auto const &scope, auto const &args)
            {
              auto ret
              (
                interpret::expect::type<interpret::cell::type::real>
                (interpret::environment::resolve_value(scope, args[0].cell)).data
              );
              ret *= interpret::expect::type<interpret::cell::type::real>
              (interpret::environment::resolve_value(scope, args[1].cell)).data;
              return environment::builtin::value::real(ret);
            }
          );
          detail::make_operator
          (
            scope, "*",
            environment::builtin::type::string(*scope),
            environment::builtin::type::integer(*scope),
            environment::builtin::type::string(*scope),
            [](auto const &scope, auto const &args)
            {
              auto const data
              (
                interpret::expect::type<interpret::cell::type::string>
                (interpret::environment::resolve_value(scope, args[0].cell)).data
              );
              auto const count
              (
                interpret::expect::type<interpret::cell::type::integer>
                (interpret::environment::resolve_value(scope, args[1].cell)).data
              );

              /* TODO: Exception type for this sort of thing? */
              if(count < 0)
              { throw expect::error::type::exception<>{ "invalid product of string" }; }

              interpret::cell::string::type ret{};
              for(auto i(0); i < count; ++i)
              { ret += data; }
              return environment::builtin::value::string(std::move(ret));
            }
          );
        }
      }
    }
  }
}
