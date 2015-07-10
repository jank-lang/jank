#include <jank/parse/expect/type.hpp>
#include <jank/translate/plugin/arithmetic/add.hpp>
#include <jank/translate/plugin/arithmetic/make_binary_operator.hpp>
#include <jank/interpret/environment/resolve_value.hpp>

namespace jank
{
  namespace translate
  {
    namespace plugin
    {
      namespace arithmetic
      {
        void add(std::shared_ptr<environment::scope> const &scope)
        {
          /* TODO: Add op+ and * and / and - for all primitives. */
          make_binary_operator
          (
            scope, "+", environment::builtin::type::integer(*scope),
            [](auto const &scope, auto const &args)
            {
              parse::cell::integer::type ret{};
              for(auto const &arg : args)
              {
                ret += parse::expect::type<parse::cell::type::integer>
                (interpret::resolve_value(scope, arg.cell)).data;
              }
              return environment::builtin::value::integer(ret);
            }
          );
          make_binary_operator
          (
            scope, "+", environment::builtin::type::real(*scope),
            [](auto const &scope, auto const &args)
            {
              parse::cell::real::type ret{};
              for(auto const &arg : args)
              {
                ret += parse::expect::type<parse::cell::type::real>
                (interpret::resolve_value(scope, arg.cell)).data;
              }
              return environment::builtin::value::real(ret);
            }
          );
          make_binary_operator
          (
            scope, "+", environment::builtin::type::string(*scope),
            [](auto const &scope, auto const &args)
            {
              parse::cell::string::type ret{};
              for(auto const &arg : args)
              {
                ret += parse::expect::type<parse::cell::type::string>
                (interpret::resolve_value(scope, arg.cell)).data;
              }
              return environment::builtin::value::string(ret);
            }
          );
        }
      }
    }
  }
}
