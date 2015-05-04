#include <jank/translate/function/argument/call.hpp>

#include <jank/translate/environment/scope.hpp>
#include <jank/translate/expect/type.hpp>
#include <jank/translate/expect/error/internal/unimplemented.hpp>

namespace jank
{
  namespace translate
  {
    namespace function
    {
      namespace argument
      {
        namespace call
        {
          value_list parse
          (
            parse::cell::list const &l,
            environment::scope const &/*scope*/
          )
          {
            value_list ret;

            std::transform
            (
              l.data.begin(), l.data.end(), std::back_inserter(ret),
              [](auto const &a) -> detail::argument_value
              {
                /* TODO: Read type from scope. */
                return parse::cell::visit
                (
                  a, [](auto const &c) -> detail::argument_value
                  {
                    if(std::is_same<decltype(c), parse::cell::ident>::value)
                    { throw expect::error::internal::unimplemented{ "ident in function call" }; }
                    else
                    {
                      throw expect::error::internal::unimplemented{ "literal value" };
                    }
                  }
                );
              }
            );

            return ret;
          }
        }
      }
    }
  }
}
