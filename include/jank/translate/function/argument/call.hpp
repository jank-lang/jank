#pragma once

#include <jank/translate/function/argument/definition.hpp>

namespace jank
{
  namespace translate
  {
    namespace function
    {
      namespace argument
      {
        namespace detail
        {
          struct argument_value
          {
            argument_type type;
            translate::cell::cell cell;
          };
          using value_list = std::vector<argument_value>;
        }
        using value_list = detail::value_list;

        namespace call
        {
          template <typename Scope>
          value_list parse
          (
            parse::cell::list const &l,
            Scope const &/*scope*/
          )
          {
            value_list ret;

            std::transform
            (
              l.data.begin(), l.data.end(), std::back_inserter(ret),
              [](auto const &a)
              {
                /* TODO: Read type from scope. */
                return parse::cell::visit
                (
                  a, [](auto const &c)
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
