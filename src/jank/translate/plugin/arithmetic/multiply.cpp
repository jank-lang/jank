#include <jank/parse/expect/type.hpp>
#include <jank/translate/plugin/arithmetic/multiply.hpp>
#include <jank/translate/plugin/arithmetic/detail/make_binary_operator.hpp>
#include <jank/interpret/environment/resolve_value.hpp>

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
          detail::make_binary_operator
          (
            scope, "*", environment::builtin::type::integer(*scope),
            [](auto const &scope, auto const &args)
            {
              auto ret
              (
                parse::expect::type<parse::cell::type::integer>
                (interpret::resolve_value(scope, args[0].cell)).data
              );
              ret *= parse::expect::type<parse::cell::type::integer>
              (interpret::resolve_value(scope, args[1].cell)).data;
              return environment::builtin::value::integer(ret);
            }
          );
          detail::make_binary_operator
          (
            scope, "*", environment::builtin::type::real(*scope),
            [](auto const &scope, auto const &args)
            {
              auto ret
              (
                parse::expect::type<parse::cell::type::real>
                (interpret::resolve_value(scope, args[0].cell)).data
              );
              ret *= parse::expect::type<parse::cell::type::real>
              (interpret::resolve_value(scope, args[1].cell)).data;
              return environment::builtin::value::real(ret);
            }
          );
          detail::make_binary_operator
          (
            scope, "*",
            environment::builtin::type::string(*scope),
            environment::builtin::type::integer(*scope),
            environment::builtin::type::string(*scope),
            [](auto const &scope, auto const &args)
            {
              auto const data
              (
                parse::expect::type<parse::cell::type::string>
                (interpret::resolve_value(scope, args[0].cell)).data
              );
              auto const count
              (
                parse::expect::type<parse::cell::type::integer>
                (interpret::resolve_value(scope, args[1].cell)).data
              );

              /* TODO: Exception type for this sort of thing? */
              if(count < 0)
              { throw expect::error::type::exception<>{ "invalid product of string" }; }

              parse::cell::string::type ret{};
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
