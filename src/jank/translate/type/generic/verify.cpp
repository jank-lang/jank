#include <jank/translate/type/generic/verify.hpp>
#include <jank/translate/expect/error/type/invalid_generic.hpp>

namespace jank
{
  namespace translate
  {
    namespace type
    {
      namespace generic
      {
        void verify
        (
          genericity<cell::detail::type_definition> const &expected,
          genericity<cell::detail::type_definition> const &provided
        )
        {
          if(expected.parameters.size() != provided.parameters.size())
          {
            throw expect::error::type::invalid_generic
            { "missing generic parameters" };
          }

          auto const equal
          (
            std::equal
            (
              expected.parameters.begin(), expected.parameters.end(),
              provided.parameters.begin(),
              [](auto const &lhs, auto const &rhs)
              { return lhs.which() == rhs.which(); }
            )
          );
          if(!equal)
          {
            throw expect::error::type::invalid_generic
            { "incorrect generic parameters" };
          }
        }
      }
    }
  }
}
